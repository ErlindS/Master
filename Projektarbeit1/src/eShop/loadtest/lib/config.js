// Central configuration. Every value can be overridden with -e NAME=value.
export const CATALOG_URL = __ENV.CATALOG_URL || 'http://localhost:5222';
export const WEBAPP_URL = __ENV.WEBAPP_URL || 'https://localhost:7298';
export const ORDERING_URL = __ENV.ORDERING_URL || 'http://localhost:5224';
export const IDENTITY_URL = __ENV.IDENTITY_URL || 'https://localhost:5243';
export const BASKET_GRPC = __ENV.BASKET_GRPC || 'localhost:5221';

// Test users seeded by Identity.API (UsersSeed.cs).
export const USERS = (__ENV.USERS || 'alice:Pass123$,bob:Pass123$')
  .split(',')
  .map((u) => {
    const [username, password] = u.split(':');
    return { username, password };
  });

// Share of sessions that end with an order (TPC-W shopping mix: 80 % browse / 20 % order).
export const ORDER_SHARE = parseFloat(__ENV.ORDER_SHARE || '0.2');

// Think time between page views in seconds (0 = closed loop without pauses).
export const THINK_TIME = parseFloat(__ENV.THINK_TIME || '0.5');

// Whether browse sessions also request server-rendered WebApp pages (WebApp -> Catalog.API).
export const INCLUDE_WEBAPP = (__ENV.INCLUDE_WEBAPP || 'true') === 'true';
