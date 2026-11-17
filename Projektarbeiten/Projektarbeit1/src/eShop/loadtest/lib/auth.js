// Obtains access tokens from Identity.API (Duende IdentityServer) using the
// authorization code flow with PKCE of the "maui" client, by filling in the login form.
import http from 'k6/http';
import crypto from 'k6/crypto';
import encoding from 'k6/encoding';
import { IDENTITY_URL } from './config.js';

const CLIENT_ID = 'maui';
const CLIENT_SECRET = 'secret';
const REDIRECT_URI = 'maui://authcallback';
const SCOPES = 'openid profile orders basket';

function randomString(length) {
  const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~';
  let s = '';
  for (let i = 0; i < length; i++) s += chars.charAt(Math.floor(Math.random() * chars.length));
  return s;
}

function formEncode(obj) {
  return Object.keys(obj)
    .map((k) => `${encodeURIComponent(k)}=${encodeURIComponent(obj[k])}`)
    .join('&');
}

function absolute(location) {
  return location.startsWith('http') ? location : `${IDENTITY_URL}${location}`;
}

export function decodeJwt(token) {
  return JSON.parse(encoding.b64decode(token.split('.')[1], 'rawurl', 's'));
}

/**
 * Logs in a user and returns { accessToken, sub, username }.
 * Uses its own cookie jar so that consecutive logins of different users do not share a session.
 */
export function login(username, password) {
  const jar = new http.CookieJar();
  const params = { jar, redirects: 0, tags: { name: 'identity-login' } };

  const verifier = randomString(64);
  const challenge = crypto.sha256(verifier, 'base64rawurl');
  const state = randomString(16);

  const authorizeUrl = `${IDENTITY_URL}/connect/authorize?` + formEncode({
    client_id: CLIENT_ID,
    response_type: 'code',
    scope: SCOPES,
    redirect_uri: REDIRECT_URI,
    code_challenge: challenge,
    code_challenge_method: 'S256',
    state,
  });

  // 1. authorize -> redirect to the login page
  let res = http.get(authorizeUrl, params);
  let guard = 0;
  while (res.status >= 300 && res.status < 400 && guard++ < 5) {
    res = http.get(absolute(res.headers.Location), params);
  }

  const doc = res.html();
  const antiforgery = doc.find('input[name=__RequestVerificationToken]').attr('value');
  const returnUrl = doc.find('input[name=ReturnUrl]').attr('value');
  if (!antiforgery || !returnUrl) {
    throw new Error(`Login page not found for ${username} (status ${res.status}, url ${res.url})`);
  }

  // 2. submit the login form
  res = http.post(res.url, {
    Username: username,
    Password: password,
    ReturnUrl: returnUrl,
    RememberLogin: 'false',
    button: 'login',
    __RequestVerificationToken: antiforgery,
  }, params);

  // For native clients the login returns an HTML page with a meta refresh instead of a 302.
  if (res.status === 200) {
    const next = res.html().find('meta[http-equiv=refresh]').attr('data-url');
    if (!next) {
      throw new Error(`Login failed for ${username} – wrong password? (url ${res.url})`);
    }
    res = http.get(absolute(next), params);
  }

  // 3. follow redirects until IdentityServer sends us to the (non-http) redirect URI with the code
  guard = 0;
  while (res.status >= 300 && res.status < 400 && guard++ < 5) {
    const location = res.headers.Location;
    if (location.startsWith(REDIRECT_URI)) {
      const code = /[?&]code=([^&]+)/.exec(location);
      if (!code) throw new Error(`No code in redirect for ${username}: ${location}`);
      return exchangeCode(decodeURIComponent(code[1]), verifier, username);
    }
    res = http.get(absolute(location), params);
  }

  throw new Error(`Login failed for ${username} (status ${res.status}, url ${res.url}) – wrong password?`);
}

function exchangeCode(code, verifier, username) {
  const res = http.post(`${IDENTITY_URL}/connect/token`, {
    grant_type: 'authorization_code',
    code,
    redirect_uri: REDIRECT_URI,
    client_id: CLIENT_ID,
    client_secret: CLIENT_SECRET,
    code_verifier: verifier,
  }, { tags: { name: 'identity-token' } });

  if (res.status !== 200) {
    throw new Error(`Token request failed for ${username}: ${res.status} ${res.body}`);
  }
  const accessToken = res.json('access_token');
  return { accessToken, sub: decodeJwt(accessToken).sub, username };
}
