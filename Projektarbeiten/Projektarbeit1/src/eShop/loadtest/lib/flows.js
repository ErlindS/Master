// User sessions: "browse" (anonymous catalog browsing) and "order" (basket + checkout).
import http from 'k6/http';
import grpc from 'k6/net/grpc';
import { check, group, sleep, fail } from 'k6';
import { Counter, Rate, Trend } from 'k6/metrics';
import { uuidv4 } from './util.js';
import {
  CATALOG_URL, WEBAPP_URL, ORDERING_URL, BASKET_GRPC, THINK_TIME, INCLUDE_WEBAPP,
} from './config.js';

// ---- custom metrics ---------------------------------------------------------------------
export const sessions = new Counter('eshop_sessions');                 // tagged with type=browse|order
export const ordersCreated = new Counter('eshop_orders_created');
export const orderSuccess = new Rate('eshop_order_success');
export const sessionDuration = new Trend('eshop_session_duration', true); // tagged with type

// ---- gRPC client for Basket.API (proto loaded once in init context) ---------------------
const basketClient = new grpc.Client();
// path is resolved relative to the main script (loadtest/eshop.js)
basketClient.load(['../src/Basket.API/Proto'], 'basket.proto');
let basketConnected = false;

function think() {
  if (THINK_TIME > 0) sleep(THINK_TIME * (0.5 + Math.random())); // +-50 % jitter
}

function pick(arr) {
  return arr[Math.floor(Math.random() * arr.length)];
}

const v = 'api-version=1.0';

// ---- browse -----------------------------------------------------------------------------
export function browse(data) {
  const start = Date.now();
  group('browse', () => {
    if (INCLUDE_WEBAPP) {
      const home = http.get(`${WEBAPP_URL}/`, { tags: { name: 'webapp GET /' } });
      check(home, { 'webapp home 200': (r) => r.status === 200 });
      think();
    }

    const pages = Math.ceil(data.items.length / 10);
    const list = http.get(`${CATALOG_URL}/api/catalog/items?${v}&pageSize=10&pageIndex=${Math.floor(Math.random() * pages)}`,
      { tags: { name: 'catalog GET /items' } });
    check(list, { 'catalog items 200': (r) => r.status === 200 });
    think();

    // filter by type and brand like the shop's filter bar
    const filtered = http.get(`${CATALOG_URL}/api/catalog/items/type/${pick(data.typeIds)}/brand/${pick(data.brandIds)}?${v}`,
      { tags: { name: 'catalog GET /items/type/{t}/brand/{b}' } });
    check(filtered, { 'catalog filter 200': (r) => r.status === 200 });
    think();

    // look at 1–3 product details (+ picture)
    const n = 1 + Math.floor(Math.random() * 3);
    for (let i = 0; i < n; i++) {
      const item = pick(data.items);
      const detail = http.get(`${CATALOG_URL}/api/catalog/items/${item.id}?${v}`, { tags: { name: 'catalog GET /items/{id}' } });
      check(detail, { 'catalog item 200': (r) => r.status === 200 });
      const pic = http.get(`${CATALOG_URL}/api/catalog/items/${item.id}/pic?${v}`, { tags: { name: 'catalog GET /items/{id}/pic' } });
      check(pic, { 'catalog pic 200': (r) => r.status === 200 });
      if (INCLUDE_WEBAPP) {
        const page = http.get(`${WEBAPP_URL}/item/${item.id}`, { tags: { name: 'webapp GET /item/{id}' } });
        check(page, { 'webapp item 200': (r) => r.status === 200 });
      }
      think();
    }
  });
  sessions.add(1, { type: 'browse' });
  sessionDuration.add(Date.now() - start, { type: 'browse' });
}

// ---- order ------------------------------------------------------------------------------
function grpcCall(method, payload, token) {
  const res = basketClient.invoke(`BasketApi.Basket/${method}`, payload, {
    metadata: { authorization: `Bearer ${token}` },
    tags: { name: `basket ${method}` },
  });
  check(res, { [`basket ${method} OK`]: (r) => r && r.status === grpc.StatusOK });
  return res;
}

export function order(data) {
  const start = Date.now();
  const user = pick(data.users);
  const token = user.accessToken;
  const authHeaders = { Authorization: `Bearer ${token}` };
  let ok = false;

  if (!basketConnected) {
    basketClient.connect(BASKET_GRPC, { plaintext: true, timeout: '10s' });
    basketConnected = true;
  }

  group('order', () => {
    // pick 1–3 products and look at them
    const chosen = [];
    const count = 1 + Math.floor(Math.random() * 3);
    for (let i = 0; i < count; i++) {
      const item = pick(data.items);
      if (chosen.find((c) => c.item.id === item.id)) continue;
      const res = http.get(`${CATALOG_URL}/api/catalog/items/${item.id}?${v}`, { tags: { name: 'catalog GET /items/{id}' } });
      if (!check(res, { 'catalog item 200': (r) => r.status === 200 })) return;
      chosen.push({ item: res.json(), quantity: 1 + Math.floor(Math.random() * 2) });
      think();
    }

    // add to basket (gRPC) and read it back
    grpcCall('UpdateBasket', { items: chosen.map((c) => ({ product_id: c.item.id, quantity: c.quantity })) }, token);
    think();
    grpcCall('GetBasket', {}, token);

    // checkout page needs the card types
    const cardTypes = http.get(`${ORDERING_URL}/api/orders/cardtypes?${v}`,
      { headers: authHeaders, tags: { name: 'ordering GET /cardtypes' } });
    check(cardTypes, { 'ordering cardtypes 200': (r) => r.status === 200 });
    think();

    const created = createOrder(user, chosen.map((c) => c.item), chosen.map((c) => c.quantity));
    ok = check(created, { 'ordering create 200': (r) => r.status === 200 });
    if (ok) ordersCreated.add(1);

    // basket is emptied after checkout, then the user looks at the order list
    grpcCall('DeleteBasket', {}, token);
    think();
    const mine = http.get(`${ORDERING_URL}/api/orders?${v}`, { headers: authHeaders, tags: { name: 'ordering GET /orders' } });
    check(mine, { 'ordering list 200': (r) => r.status === 200 });
  });

  orderSuccess.add(ok);
  sessions.add(1, { type: 'order' });
  sessionDuration.add(Date.now() - start, { type: 'order' });
}

// Places an order with the same payload the WebApp sends in BasketState.CheckoutAsync.
export function createOrder(user, items, quantities) {
  const body = {
    userId: user.sub,
    userName: user.username,
    city: 'Redmond',
    street: '15703 NE 61st Ct',
    state: 'WA',
    country: 'U.S.',
    zipCode: '98052',
    cardNumber: '1111222233334444',
    cardHolderName: 'TESTUSER',
    cardExpiration: new Date(Date.now() + 365 * 24 * 3600 * 1000).toISOString(),
    cardSecurityNumber: '111',
    cardTypeId: 1,
    buyer: user.sub,
    items: items.map((item, i) => ({
      id: String(item.id),
      productId: item.id,
      productName: item.name,
      unitPrice: item.price,
      oldUnitPrice: 0,
      quantity: quantities[i],
      pictureUrl: `${item.id}.webp`,
    })),
  };
  return http.post(`${ORDERING_URL}/api/orders?${v}`, JSON.stringify(body), {
    headers: {
      'Content-Type': 'application/json',
      'x-requestid': uuidv4(),
      Authorization: `Bearer ${user.accessToken}`,
    },
    tags: { name: 'ordering POST /orders' },
  });
}

/**
 * Ordering.API creates the buyer on a user's first order. Concurrent first orders of the same user
 * collide on the unique index IX_buyers_IdentityGuid, so every user places one order up front.
 */
export function ensureBuyers(users) {
  const item = http.get(`${CATALOG_URL}/api/catalog/items/1?${v}`, { tags: { name: 'setup' } }).json();
  for (const user of users) {
    const res = createOrder(user, [item], [1]);
    if (res.status !== 200) fail(`Initial order for ${user.username} failed: ${res.status} ${res.body}`);
  }
}

// ---- data used by all VUs (loaded once in setup) ------------------------------------------
export function loadCatalog() {
  const res = http.get(`${CATALOG_URL}/api/catalog/items?${v}&pageSize=200&pageIndex=0`, { tags: { name: 'setup' } });
  if (res.status !== 200) fail(`Catalog.API not reachable at ${CATALOG_URL} (status ${res.status}) – is the AppHost running?`);
  const items = res.json('data').map((i) => ({ id: i.id }));
  const typeIds = http.get(`${CATALOG_URL}/api/catalog/catalogtypes?${v}`, { tags: { name: 'setup' } }).json().map((t) => t.id);
  const brandIds = http.get(`${CATALOG_URL}/api/catalog/catalogbrands?${v}`, { tags: { name: 'setup' } }).json().map((b) => b.id);
  return { items, typeIds, brandIds };
}
