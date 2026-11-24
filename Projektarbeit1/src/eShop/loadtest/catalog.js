// k6 load test for the eShop (run while the Aspire AppHost is running).
//   k6 run loadtest/catalog.js
//   k6 run -e CATALOG_URL=http://localhost:5222 -e WEBAPP_URL=https://localhost:7298 loadtest/catalog.js
import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  insecureSkipTLSVerify: true, // ASP.NET Core dev certificate
  stages: [
    { duration: '30s', target: 100 }, // ramp up to 10 virtual users
    { duration: '1m', target: 100 },
    { duration: '30s', target: 0 },
  ],
  thresholds: {
    http_req_failed: ['rate<0.05'],
    http_req_duration: ['p(95)<1000'],
  },
};

const CATALOG = __ENV.CATALOG_URL || 'http://localhost:5222';
const WEBAPP = __ENV.WEBAPP_URL || 'https://localhost:7298';

export default function () {
  // Web shop start page (WebApp -> Catalog.API, Basket.API)
  let res = http.get(`${WEBAPP}/`, { tags: { name: 'webapp-home' } });
  check(res, { 'home 200': (r) => r.status === 200 });

  // Catalog API: paged list
  const page = Math.floor(Math.random() * 5);
  res = http.get(`${CATALOG}/api/catalog/items?api-version=1.0&pageSize=10&pageIndex=${page}`,
    { tags: { name: 'catalog-items' } });
  check(res, { 'items 200': (r) => r.status === 200 });

  // Catalog API: single item
  const id = Math.floor(Math.random() * 100) + 1;
  res = http.get(`${CATALOG}/api/catalog/items/${id}?api-version=1.0`,
    { tags: { name: 'catalog-item-by-id' } });
  check(res, { 'item 200/404': (r) => r.status === 200 || r.status === 404 });

  // Catalog API: brands and types
  res = http.get(`${CATALOG}/api/catalog/catalogBrands?api-version=1.0`, { tags: { name: 'catalog-brands' } });
  check(res, { 'brands 200': (r) => r.status === 200 });

  sleep(1);
}
