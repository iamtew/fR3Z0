// PixiJS backdrop: Chladni plate, full-viewport, no CDN.
const C = { magenta: 0xE038E9, cyan: 0x2FC1EE, white: 0xF0F8FF };
const PI = Math.PI;
const HOLD = 7;     // seconds a figure sits so sand can settle
const FADE = 1.5;   // crossfade into the next catalog entry
const GX = 64;      // nodal grid, cells
const GY = 36;
const NODE_EPS = 1e-6;
const reduceMotion = matchMedia('(prefers-reduced-motion: reduce)').matches;
const rand = (lo, hi) => lo + Math.random() * (hi - lo);
const clamp = (v, lo, hi) => Math.min(hi, Math.max(lo, v));
const smooth = (k) => k * k * (3 - 2 * k);

// Distinct topologies: cells, stripes, crosses, diamonds — not a slow drift of one plate.
const PATTERNS = [
  { n1: 4, m1: 4, n2: 2, m2: 6, a1: 1, a2: 0.22 },
  { n1: 1, m1: 1, n2: 5, m2: 1, a1: 1, a2: 0.28 },
  { n1: 3, m1: 5, n2: 4, m2: 2, a1: 1, a2: 0.22 },
  { n1: 3, m1: 1, n2: 1, m2: 5, a1: 1, a2: 0.26 },
  { n1: 5, m1: 3, n2: 6, m2: 2, a1: 1, a2: 0.20 },
  { n1: 2, m1: 5, n2: 7, m2: 1, a1: 1, a2: 0.24 },
  { n1: 6, m1: 4, n2: 3, m2: 3, a1: 1, a2: 0.22 },
  { n1: 2, m1: 3, n2: 1, m2: 4, a1: 1, a2: 0.22 },
];

// ?fx / ?fx=true keeps only the canvas. ?fx=false / 0 / off stays a normal page.
const fxRaw = new URLSearchParams(location.search).get('fx');
const fxOnly = fxRaw !== null && !/^(0|false|off)$/i.test(fxRaw);
if (fxOnly) document.body.classList.add('fx-only');

const pointer = { x: 0.5, y: 0.5 };
if (!fxOnly) {
  addEventListener('pointermove', (e) => {
    pointer.x = e.clientX / innerWidth;
    pointer.y = e.clientY / innerHeight;
  }, { passive: true });
}

start().catch((err) => {
  console.info('fx: animated backdrop disabled:', err?.message || err);
  document.getElementById('fx')?.remove();
});

async function start() {
  const PIXI = await import('./vendor/pixi.min.mjs');
  const app = new PIXI.Application();
  await app.init({
    canvas: document.getElementById('fx'),
    resizeTo: window,
    backgroundAlpha: 0,
    antialias: true,
    autoDensity: true,
    resolution: Math.min(devicePixelRatio || 1, 2),
    preference: 'webgl',
  });

  const glow = glowTexture(PIXI);
  const scene = chladniScene(PIXI, glow);
  app.stage.addChild(scene.root);
  bootTicker(app, scene);
}

function bootTicker(app, scene) {
  if (reduceMotion) {
    app.ticker.stop();
    const still = () => {
      const { width: w, height: h } = app.screen;
      scene.update(HOLD * 0.5, w, h, 0);
      app.render();
    };
    addEventListener('resize', () => requestAnimationFrame(still));
    still();
    return;
  }

  let t = 0;
  app.ticker.add((ticker) => {
    const dt = Math.min(ticker.deltaMS / 1000, 0.05);
    t += dt;
    const { width: w, height: h } = app.screen;
    // Preview mode is full-bleed, so there is no page to parallax against.
    if (!fxOnly) {
      const ease = Math.min(1, dt * 2.5);
      scene.root.x += ((0.5 - pointer.x) * 40 - scene.root.x) * ease;
      scene.root.y += ((0.5 - pointer.y) * 24 - scene.root.y) * ease;
    }
    scene.update(t, w, h, dt);
  });

  document.addEventListener('visibilitychange', () => {
    if (document.hidden) app.ticker.stop(); else app.ticker.start();
  });
}

function glowTexture(PIXI, size = 64) {
  const c = document.createElement('canvas');
  c.width = c.height = size;
  const g = c.getContext('2d');
  const r = size / 2;
  const grd = g.createRadialGradient(r, r, 0, r, r, r);
  grd.addColorStop(0, 'rgba(255,255,255,1)');
  grd.addColorStop(0.25, 'rgba(255,255,255,0.5)');
  grd.addColorStop(1, 'rgba(255,255,255,0)');
  g.fillStyle = grd;
  g.fillRect(0, 0, size, size);
  return PIXI.Texture.from(c);
}

function glowSprite(PIXI, tex, tint) {
  const s = new PIXI.Sprite(tex);
  s.anchor.set(0.5);
  s.tint = tint;
  s.blendMode = 'add';
  return s;
}

const NODE_PASSES = [[18, 0.08], [8, 0.18], [3.2, 0.45], [1.3, 1]];
const ISO_PASSES = [[5, 0.08], [1.4, 0.32]];
const TRACE_PASSES = [[6, 0.08], [1.8, 0.28]];
const TRAIL = 8;
function glowSegs(g, segs, color, scale = 1, passes = NODE_PASSES) {
  if (!segs.length) return;
  for (const [width, alpha] of passes) {
    for (let i = 0; i < segs.length; i += 4) {
      g.moveTo(segs[i], segs[i + 1]);
      g.lineTo(segs[i + 2], segs[i + 3]);
    }
    g.stroke({ width: width * scale, color, alpha, cap: 'round' });
  }
}

function pairU(p, x, y, beat) {
  return p.a1 * Math.cos(p.n1 * PI * x) * Math.cos(p.m1 * PI * y)
    + p.a2 * beat * Math.cos(p.n2 * PI * x) * Math.cos(p.m2 * PI * y);
}

function ringEnv(t) {
  return 0.88 + 0.12 * Math.exp(-((t % 4.2) / 4.2) * 3.4);
}

function fieldAt(t, x, y) {
  const period = HOLD + FADE;
  const n = PATTERNS.length;
  const slot = ((t / period) % n + n) % n;
  const i = Math.floor(slot);
  const k = smooth(clamp((slot - i - HOLD / period) / (FADE / period), 0, 1));
  // Beat stays off zero so topology holds during a figure; envelope is a light ring.
  const beat = 0.78 + 0.22 * Math.cos(t * 1.25);
  const a = pairU(PATTERNS[i], x, y, beat);
  const b = pairU(PATTERNS[(i + 1) % n], x, y, beat);
  return ringEnv(t) * (a + (b - a) * k);
}

function pushContour(grid, nx, w, h, segs, at) {
  const cellW = w / GX;
  const cellH = h / GY;
  for (let j = 0; j < GY; j++) {
    for (let i = 0; i < GX; i++) {
      const ua = at(grid[j * nx + i]);
      const ub = at(grid[j * nx + i + 1]);
      const uc = at(grid[(j + 1) * nx + i + 1]);
      const ud = at(grid[(j + 1) * nx + i]);
      const x0 = i * cellW, y0 = j * cellH;
      const x1 = x0 + cellW, y1 = y0 + cellH;
      const hits = [];
      if (ua * ub < 0) hits.push(lerp2(x0, y0, x1, y0, ua, ub));
      if (ub * uc < 0) hits.push(lerp2(x1, y0, x1, y1, ub, uc));
      if (ud * uc < 0) hits.push(lerp2(x0, y1, x1, y1, ud, uc));
      if (ua * ud < 0) hits.push(lerp2(x0, y0, x0, y1, ua, ud));
      if (hits.length >= 2) {
        segs.push(hits[0][0], hits[0][1], hits[1][0], hits[1][1]);
        if (hits.length === 4) segs.push(hits[2][0], hits[2][1], hits[3][0], hits[3][1]);
      }
    }
  }
}

function chladniScene(PIXI, glow) {
  const root = new PIXI.Container();
  const haze = new PIXI.Graphics();
  const nodes = new PIXI.Graphics();
  const traces = new PIXI.Graphics();
  haze.blendMode = 'add';
  nodes.blendMode = 'add';
  traces.blendMode = 'add';

  const bloom = glowSprite(PIXI, glow, C.magenta);
  bloom.anchor.set(0.5);

  const sand = Array.from({ length: 180 }, (_, i) => {
    const tint = i % 5 === 0 ? C.magenta : (i % 3 === 0 ? C.cyan : C.white);
    const s = glowSprite(PIXI, glow, tint);
    s.nx = rand(0.03, 0.97);
    s.ny = rand(0.03, 0.97);
    s.vx = 0;
    s.vy = 0;
    s.size = i % 9 === 0 ? rand(0.28, 0.45) : rand(0.12, 0.22);
    s.trail = [];
    return s;
  });

  root.addChild(haze, nodes, traces, ...sand, bloom);
  root.alpha = fxOnly ? 1 : 0.62;

  const grid = new Float32Array((GX + 1) * (GY + 1));
  const zeroSegs = [];
  const isoLo = [];
  const isoHi = [];
  const trailSegs = [];

  return {
    root,
    update(t, w, h, dt) {
      const nx = GX + 1;
      const ny = GY + 1;
      for (let j = 0; j < ny; j++) {
        const y = j / GY;
        for (let i = 0; i < nx; i++) grid[j * nx + i] = fieldAt(t, i / GX, y);
      }

      const cellW = w / GX;
      const cellH = h / GY;
      const env = ringEnv(t);
      haze.clear();
      for (let j = 0; j < GY; j++) {
        for (let i = 0; i < GX; i++) {
          const ua = grid[j * nx + i];
          const ub = grid[j * nx + i + 1];
          const uc = grid[(j + 1) * nx + i + 1];
          const ud = grid[(j + 1) * nx + i];
          const avg = (ua + ub + uc + ud) * 0.25;
          const mag = Math.abs(avg);
          if (mag < 0.04) continue;
          haze.rect(i * cellW, j * cellH, cellW + 0.5, cellH + 0.5)
            .fill({ color: avg >= 0 ? C.magenta : C.cyan, alpha: mag * 0.22 * env });
        }
      }

      zeroSegs.length = 0;
      isoLo.length = 0;
      isoHi.length = 0;
      pushContour(grid, nx, w, h, zeroSegs, (v) => v);
      pushContour(grid, nx, w, h, isoLo, (v) => Math.abs(v) - 0.35);
      pushContour(grid, nx, w, h, isoHi, (v) => Math.abs(v) - 0.6);

      nodes.clear();
      glowSegs(nodes, isoHi, C.cyan, 0.7, ISO_PASSES);
      glowSegs(nodes, isoLo, C.magenta, 0.85, ISO_PASSES);
      glowSegs(nodes, zeroSegs, C.magenta, 1.15);
      glowSegs(nodes, zeroSegs, C.cyan, 0.6, [[10, 0.08], [2.2, 0.35]]);
      glowSegs(nodes, zeroSegs, C.white, 0.55, [[1.2, 0.7]]);

      bloom.x = w * 0.5;
      bloom.y = h * 0.5;
      bloom.scale.set((Math.min(w, h) * 0.055) / 8, (Math.min(w, h) * 0.04) / 8);
      bloom.alpha = 0.12 + 0.18 * ((env - 0.88) / 0.12);

      trailSegs.length = 0;
      const e = 0.018;
      for (const s of sand) {
        if (dt > 0) {
          const u = Math.abs(fieldAt(t, s.nx, s.ny));
          const ux = Math.abs(fieldAt(t, s.nx + e, s.ny)) - Math.abs(fieldAt(t, s.nx - e, s.ny));
          const uy = Math.abs(fieldAt(t, s.nx, s.ny + e)) - Math.abs(fieldAt(t, s.nx, s.ny - e));
          // Down |u|: leave antinodes, pile on nodes.
          s.vx = s.vx * 0.86 - ux * 2.8;
          s.vy = s.vy * 0.86 - uy * 2.8;
          s.vx += rand(-0.0015, 0.0015);
          s.vy += rand(-0.0015, 0.0015);
          s.nx = clamp(s.nx + s.vx * dt, 0.02, 0.98);
          s.ny = clamp(s.ny + s.vy * dt, 0.02, 0.98);
          // Traces only once a grain has found a node, so they don't web the antinodes.
          if (u < 0.22) {
            s.trail.push(s.nx, s.ny);
            if (s.trail.length > TRAIL * 2) s.trail.splice(0, 2);
          } else if (s.trail.length) {
            s.trail.length = 0;
          }
          s.alpha = 0.22 + 0.7 * clamp(1 - u * 2.2, 0, 1);
        } else {
          s.alpha = 0.55;
        }
        s.x = s.nx * w;
        s.y = s.ny * h;
        s.scale.set(s.size, s.size);
        const tr = s.trail;
        for (let k = 2; k < tr.length; k += 2) {
          const dx = tr[k] - tr[k - 2], dy = tr[k + 1] - tr[k - 1];
          if (dx * dx + dy * dy > 0.004) continue;
          trailSegs.push(tr[k - 2] * w, tr[k - 1] * h, tr[k] * w, tr[k + 1] * h);
        }
      }

      traces.clear();
      if (dt > 0) glowSegs(traces, trailSegs, C.white, 0.7, TRACE_PASSES);
    },
  };
}

function lerp2(x0, y0, x1, y1, u0, u1) {
  const d = u0 - u1;
  const t = Math.abs(d) < NODE_EPS ? 0.5 : u0 / d;
  return [x0 + (x1 - x0) * t, y0 + (y1 - y0) * t];
}
