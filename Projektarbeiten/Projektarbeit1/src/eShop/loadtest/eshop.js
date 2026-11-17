// eShop load test – open model with a fixed arrival rate and a TPC-W-like 80/20 browse/order mix.
//
// Run while the Aspire AppHost is running:
//   k6 run loadtest/eshop.js                                  # measurement run: 2 min warm-up, 5 min measurement, 1 min cool-down
//   k6 run -e PROFILE=smoke loadtest/eshop.js                 # 30 s functional check with low load
//   k6 run -e RATE=20 loadtest/eshop.js                       # 20 user sessions per second
//   k6 run -e WARMUP=1m -e DURATION=3m -e COOLDOWN=30s loadtest/eshop.js
//
// Other settings (see lib/config.js): ORDER_SHARE, THINK_TIME, INCLUDE_WEBAPP, USERS,
// CATALOG_URL, WEBAPP_URL, ORDERING_URL, IDENTITY_URL, BASKET_GRPC, RUN_ID, RESULTS_DIR.
import { sleep } from 'k6';
import exec from 'k6/execution';
import { textSummary } from 'https://jslib.k6.io/k6-summary/0.1.0/index.js';
import { USERS, ORDER_SHARE } from './lib/config.js';
import { login } from './lib/auth.js';
import { browse, order, loadCatalog, ensureBuyers } from './lib/flows.js';

const PROFILE = __ENV.PROFILE || 'messlauf';
const smoke = PROFILE === 'smoke';

const RATE = parseInt(__ENV.RATE || (smoke ? '2' : '10'), 10);   // new sessions per second
const WARMUP = __ENV.WARMUP || (smoke ? '0s' : '2m');
const DURATION = __ENV.DURATION || (smoke ? '30s' : '5m');
const COOLDOWN = __ENV.COOLDOWN || (smoke ? '0s' : '1m');
const MAX_VUS = parseInt(__ENV.MAX_VUS || String(Math.max(50, RATE * 20)), 10);
const RUN_ID = __ENV.RUN_ID || new Date().toISOString().replace(/[:.]/g, '-');

function seconds(d) {
  const m = /^(\d+(?:\.\d+)?)(ms|s|m|h)$/.exec(d);
  if (!m) throw new Error(`Invalid duration: ${d}`);
  return parseFloat(m[1]) * { ms: 0.001, s: 1, m: 60, h: 3600 }[m[2]];
}

function arrival(phase, duration, startTime) {
  return {
    executor: 'constant-arrival-rate',
    exec: 'session',
    rate: RATE,
    timeUnit: '1s',
    duration,
    startTime,
    preAllocatedVUs: Math.min(MAX_VUS, Math.max(10, RATE * 5)),
    maxVUs: MAX_VUS,
    gracefulStop: '30s',
    tags: { phase },
  };
}

const scenarios = {};
if (seconds(WARMUP) > 0) scenarios.warmup = arrival('warmup', WARMUP, '0s');
scenarios.measure = arrival('measure', DURATION, WARMUP);
if (seconds(COOLDOWN) > 0) {
  // no load: lets asynchronous processing (OrderProcessor, PaymentProcessor) finish and telemetry flush
  scenarios.cooldown = {
    executor: 'per-vu-iterations', exec: 'idle', vus: 1, iterations: 1,
    startTime: `${seconds(WARMUP) + seconds(DURATION) + 30}s`, maxDuration: `${seconds(COOLDOWN) + 10}s`,
    tags: { phase: 'cooldown' },
  };
}

export const options = {
  insecureSkipTLSVerify: true, // ASP.NET Core dev certificates
  scenarios,
  tags: { run_id: RUN_ID },
  // Validity criteria of a run – only evaluated for the measurement phase.
  thresholds: {
    'http_req_failed{phase:measure}': ['rate<0.01'],                 // error rate <= 1 %
    'checks{phase:measure}': ['rate>0.99'],
    'dropped_iterations{scenario:measure}': ['count==0'],            // target rate reached, k6 not saturated
    'http_req_duration{phase:measure}': ['p(95)<2000'],
    'eshop_order_success{phase:measure}': ['rate>0.99'],
    // listed only so that these sub-metrics appear in the summary (mix check)
    'eshop_sessions{type:browse}': ['count>=0'],
    'eshop_sessions{type:order}': ['count>=0'],
    'eshop_sessions{phase:measure}': ['count>0'],
    'eshop_session_duration{type:browse}': ['max>=0'],
    'eshop_session_duration{type:order}': ['max>=0'],
  },
  summaryTrendStats: ['avg', 'min', 'med', 'p(90)', 'p(95)', 'p(99)', 'max'],
};

export function setup() {
  const catalog = loadCatalog();
  const users = USERS.map((u) => login(u.username, u.password));
  ensureBuyers(users);
  console.log(`Profile=${PROFILE} rate=${RATE}/s warmup=${WARMUP} measure=${DURATION} cooldown=${COOLDOWN} ` +
    `orderShare=${ORDER_SHARE} items=${catalog.items.length} users=${users.map((u) => u.username).join(',')} run=${RUN_ID}`);
  return Object.assign(catalog, { users });
}

// One iteration = one user session. The mix is deterministic and evenly interleaved
// (with ORDER_SHARE=0.2 exactly every 5th session is an order), so every run has the same mix.
export function session(data) {
  const i = exec.scenario.iterationInTest;
  if (Math.floor((i + 1) * ORDER_SHARE) > Math.floor(i * ORDER_SHARE)) {
    order(data);
  } else {
    browse(data);
  }
}

export function idle() {
  sleep(seconds(COOLDOWN));
}

// ---- result files + mix check ------------------------------------------------------------
function metricValue(data, name, field) {
  const m = data.metrics[name];
  return m && m.values ? m.values[field] : undefined;
}

export function handleSummary(data) {
  const browseN = metricValue(data, 'eshop_sessions{type:browse}', 'count') || 0;
  const orderN = metricValue(data, 'eshop_sessions{type:order}', 'count') || 0;
  const total = browseN + orderN;
  const measuredCount = metricValue(data, 'eshop_sessions{phase:measure}', 'count');
  const measuredRate = measuredCount ? measuredCount / seconds(DURATION) : undefined;

  const mix = total > 0
    ? `Mix: browse ${(100 * browseN / total).toFixed(1)} % / order ${(100 * orderN / total).toFixed(1)} % ` +
      `(Soll ${(100 * (1 - ORDER_SHARE)).toFixed(0)}/${(100 * ORDER_SHARE).toFixed(0)}, n=${total})`
    : 'Mix: keine Sessions';
  const rate = `Rate (Messphase): ${measuredRate ? measuredRate.toFixed(2) : '-'} Sessions/s (Soll ${RATE}/s), ` +
    `verworfen: ${metricValue(data, 'dropped_iterations{scenario:measure}', 'count') || 0}`;

  const dir = __ENV.RESULTS_DIR || 'loadtest/results';
  return {
    stdout: textSummary(data, { indent: ' ', enableColors: true }) + `\n\n     ${mix}\n     ${rate}\n\n`,
    [`${dir}/${RUN_ID}-summary.json`]: JSON.stringify(data, null, 2),
  };
}
