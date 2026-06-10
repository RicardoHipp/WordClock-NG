// ###########################################################################################################################################
// # WordClock animation framework
// # Add new effects here. To register: add #define, extend animationStart() and animationTick().
// ###########################################################################################################################################

#define MATRIX_TRAIL 4    // trail length behind head (pixels)
#define ANIM_TICK_MS  80  // ms per animation frame

static unsigned long lastAnimTick = 0;

// ── Matrix ────────────────────────────────────────────────────────────────────
static int8_t matrixHead[16];
static bool   matrixColDone[16];

static uint32_t matrixColor(int dist) {
  switch (dist) {
    case 0: return strip.Color(120, 255, 120);  // head — bright green-white
    case 1: return strip.Color(  0, 200,   0);
    case 2: return strip.Color(  0, 110,   0);
    case 3: return strip.Color(  0,  50,   0);
    case 4: return strip.Color(  0,  15,   0);
    default: return 0;
  }
}

static void matrixInit() {
  for (int c = 0; c < 16; c++) {
    matrixHead[c]    = -(int8_t)random(0, 9);  // staggered start per column
    matrixColDone[c] = false;
  }
}

static void matrixTick() {
  ClearDisplay();
  int done = 0;
  for (int c = 0; c < 16; c++) {
    if (matrixColDone[c]) { done++; continue; }
    matrixHead[c]++;
    for (int r = 1; r <= 8; r++) {
      int dist = matrixHead[c] - (r - 1);
      if (dist >= 0 && dist <= MATRIX_TRAIL) {
        DrawPixel(r, c + 1, matrixColor(dist));
      }
    }
    if (matrixHead[c] >= 8 + MATRIX_TRAIL) {
      matrixColDone[c] = true;
      done++;
    }
  }
  showStrip();
  if (done == 16) {
    animationRunning = false;
    updatenow = true;
    update_display();
  }
}

// ── Shared: pixel buffers ─────────────────────────────────────────────────────
static uint32_t targetPixels[NUMPIXELS];
static uint32_t sourcePixels[NUMPIXELS];

static void snapshotTarget() {
  updatenow = true;
  update_display();                          // renders final state into strip
  for (int i = 0; i < NUMPIXELS; i++)
    targetPixels[i] = strip.getPixelColor(i);
  ClearDisplay();
  showStrip();                              // black out immediately
}

// ── Teletype ──────────────────────────────────────────────────────────────────
#define TELETYPE_TICK_MS 80                  // ms per visible character

static int teletypeCharIdx;                  // current character position (0..127)

static void teletypeInit() {
  snapshotTarget();
  teletypeCharIdx = 0;
}

static void teletypeTick() {
  // skip invisible (black) characters instantly, only pause on visible ones
  while (teletypeCharIdx < MAXROWS * MAXCOLUMS) {
    int r  = teletypeCharIdx / MAXCOLUMS;
    int c  = teletypeCharIdx % MAXCOLUMS;
    int up = r * 32 + (15 - c);
    int lo = r * 32 + (16 + c);
    if (targetPixels[up] != 0 || targetPixels[lo] != 0) break;
    teletypeCharIdx++;
  }
  if (teletypeCharIdx >= MAXROWS * MAXCOLUMS) {
    animationRunning = false;
    return;
  }
  int r  = teletypeCharIdx / MAXCOLUMS;
  int c  = teletypeCharIdx % MAXCOLUMS;
  int up = r * 32 + (15 - c);
  int lo = r * 32 + (16 + c);
  strip.setPixelColor(up, targetPixels[up]);
  strip.setPixelColor(lo, targetPixels[lo]);
  teletypeCharIdx++;
  if (teletypeCharIdx >= MAXROWS * MAXCOLUMS) {
    showStripSafe();
    animationRunning = false;
  } else {
    showStrip();
  }
}

// ── Fade ──────────────────────────────────────────────────────────────────────
#define FADE_TICK_MS  30   // ms per step
#define FADE_STEPS    40   // total steps — half fade out, half fade in

static int fadeStep;

static void fadeInit() {
  for (int i = 0; i < NUMPIXELS; i++)
    sourcePixels[i] = strip.getPixelColor(i);
  updatenow = true;
  update_display();
  for (int i = 0; i < NUMPIXELS; i++)
    targetPixels[i] = strip.getPixelColor(i);
  for (int i = 0; i < NUMPIXELS; i++)
    strip.setPixelColor(i, sourcePixels[i]);
  showStrip();
  fadeStep = 0;
}

static void fadeTick() {
  for (int i = 0; i < NUMPIXELS; i++) {
    uint32_t col;
    if (fadeStep < FADE_STEPS / 2) {
      int alpha = (FADE_STEPS / 2 - 1 - fadeStep);
      uint32_t s = sourcePixels[i];
      uint8_t r = ((s >> 16) & 0xFF) * alpha / (FADE_STEPS / 2 - 1);
      uint8_t g = ((s >>  8) & 0xFF) * alpha / (FADE_STEPS / 2 - 1);
      uint8_t b = ( s        & 0xFF) * alpha / (FADE_STEPS / 2 - 1);
      col = strip.Color(r, g, b);
    } else {
      int alpha = fadeStep - FADE_STEPS / 2;
      uint32_t t = targetPixels[i];
      uint8_t r = ((t >> 16) & 0xFF) * alpha / (FADE_STEPS / 2 - 1);
      uint8_t g = ((t >>  8) & 0xFF) * alpha / (FADE_STEPS / 2 - 1);
      uint8_t b = ( t        & 0xFF) * alpha / (FADE_STEPS / 2 - 1);
      col = strip.Color(r, g, b);
    }
    strip.setPixelColor(i, col);
  }
  fadeStep++;
  if (fadeStep >= FADE_STEPS) {
    showStripSafe();
    animationRunning = false;
  } else {
    showStrip();
  }
}

// ── Squeeze ───────────────────────────────────────────────────────────────────
#define SQUEEZE_TICK_MS 60

static int squeezePhase;  // 0 = squeeze out (right→left), 1 = squeeze in (left→right)

static inline int sqUp(int r, int c) { return r * 32 + (15 - c); }
static inline int sqLo(int r, int c) { return r * 32 + (16 + c); }

static void squeezeInit() {
  for (int i = 0; i < NUMPIXELS; i++)
    sourcePixels[i] = strip.getPixelColor(i);  // current display
  updatenow = true;
  update_display();
  for (int i = 0; i < NUMPIXELS; i++)
    targetPixels[i] = strip.getPixelColor(i);  // new time
  for (int i = 0; i < NUMPIXELS; i++)
    strip.setPixelColor(i, sourcePixels[i]);   // restore current to strip
  showStrip();
  squeezePhase = 0;
}

static void squeezeTick() {
  bool anyFound = false;

  if (squeezePhase == 0) {
    // Phase 1: remove rightmost lit char per row
    for (int r = 0; r < 8; r++) {
      for (int c = 15; c >= 0; c--) {
        if (sourcePixels[sqUp(r,c)] || sourcePixels[sqLo(r,c)]) {
          sourcePixels[sqUp(r,c)] = 0;
          sourcePixels[sqLo(r,c)] = 0;
          anyFound = true;
          break;
        }
      }
    }
    for (int i = 0; i < NUMPIXELS; i++)
      strip.setPixelColor(i, sourcePixels[i]);
    showStrip();
    if (!anyFound) {
      squeezePhase = 1;
      for (int i = 0; i < NUMPIXELS; i++) sourcePixels[i] = 0;  // reuse as "shown" buffer
    }
  }

  if (squeezePhase == 1) {
    // Phase 2: add leftmost unshown target char per row  (NOT else — fires same tick as transition)
    anyFound = false;
    for (int r = 0; r < 8; r++) {
      for (int c = 0; c < 16; c++) {
        if ((targetPixels[sqUp(r,c)] || targetPixels[sqLo(r,c)]) &&
            !sourcePixels[sqUp(r,c)] && !sourcePixels[sqLo(r,c)]) {
          sourcePixels[sqUp(r,c)] = targetPixels[sqUp(r,c)];
          sourcePixels[sqLo(r,c)] = targetPixels[sqLo(r,c)];
          anyFound = true;
          break;
        }
      }
    }
    for (int i = 0; i < NUMPIXELS; i++)
      strip.setPixelColor(i, sourcePixels[i]);
    if (!anyFound) {
      showStripSafe();
      animationRunning = false;
    } else {
      showStrip();
    }
  }
}

// ── Snake ─────────────────────────────────────────────────────────────────────
#define SNAKE_TICK_MS  20
#define SNAKE_LEN_DEF  11   // body length in cells (≈ WC_COLUMNS - 5)

static int     snakePos;
static uint32_t snakeColor;

// Convert zigzag step to (row, col) — even rows left→right, odd rows right→left
static void snakeGetRC(int step, int &r, int &c) {
  if (step < 0 || step >= 8 * 16) { r = -1; c = -1; return; }
  r = step / 16;
  int ci = step % 16;
  c = (r & 1) ? (15 - ci) : ci;
}

static void snakeInit() {
  updatenow = true;
  update_display();
  for (int i = 0; i < NUMPIXELS; i++)
    targetPixels[i] = strip.getPixelColor(i);
  ClearDisplay();
  showStrip();
  snakeColor = strip.Color(redVal_time, greenVal_time, blueVal_time);
  snakePos = 0;
}

static void snakeTick() {
  int total = 8 * 16 + SNAKE_LEN_DEF;
  if (snakePos >= total) {
    for (int i = 0; i < NUMPIXELS; i++)
      strip.setPixelColor(i, targetPixels[i]);
    showStripSafe();
    animationRunning = false;
    return;
  }
  // draw head
  int hr, hc;
  snakeGetRC(snakePos, hr, hc);
  if (hr >= 0) {
    strip.setPixelColor(sqUp(hr, hc), snakeColor);
    strip.setPixelColor(sqLo(hr, hc), snakeColor);
  }
  // reveal tail: target color if target, else black
  int tailStep = snakePos - SNAKE_LEN_DEF;
  if (tailStep >= 0) {
    int tr, tc;
    snakeGetRC(tailStep, tr, tc);
    if (tr >= 0) {
      strip.setPixelColor(sqUp(tr, tc), targetPixels[sqUp(tr, tc)]);
      strip.setPixelColor(sqLo(tr, tc), targetPixels[sqLo(tr, tc)]);
    }
  }
  showStrip();
  snakePos++;
}

// ── Flicker ───────────────────────────────────────────────────────────────────
#define FLICKER_TICK_MS  60
#define FLICKER_FRAMES   32

static int flickerCnt;

static void flickerInit() {
  for (int i = 0; i < NUMPIXELS; i++)
    sourcePixels[i] = strip.getPixelColor(i);
  updatenow = true;
  update_display();
  for (int i = 0; i < NUMPIXELS; i++)
    targetPixels[i] = strip.getPixelColor(i);
  for (int i = 0; i < NUMPIXELS; i++)
    strip.setPixelColor(i, sourcePixels[i]);
  showStrip();
  flickerCnt = 0;
}

static void flickerTick() {
  flickerCnt++;
  if (flickerCnt >= FLICKER_FRAMES) {
    for (int i = 0; i < NUMPIXELS; i++)
      strip.setPixelColor(i, targetPixels[i]);
    showStripSafe();
    animationRunning = false;
    return;
  }
  bool on = (random(8) > 1);  // 75% an, 25% aus
  for (int i = 0; i < NUMPIXELS; i++) {
    uint32_t p = targetPixels[i] ? targetPixels[i] : sourcePixels[i];
    strip.setPixelColor(i, on ? p : 0);
  }
  showStrip();
}

// ── Public API ────────────────────────────────────────────────────────────────
void animationStart() {
  animationRunning = false;  // block loop ticks during init
  switch (animationMode) {
    case ANIM_MATRIX:   matrixInit();   break;
    case ANIM_TELETYPE: teletypeInit(); break;
    case ANIM_FADE:     fadeInit();     break;
    case ANIM_SQUEEZE:  squeezeInit();  break;
    case ANIM_SNAKE:    snakeInit();    break;
    case ANIM_FLICKER:  flickerInit();  break;
  }
  lastAnimTick     = millis() - ANIM_TICK_MS;  // first tick fires immediately
  animationRunning = true;   // enable ticks only after init is complete
}

void animationTick() {
  if (!animationRunning) return;
  unsigned long tickMs = (animationMode == ANIM_TELETYPE) ? TELETYPE_TICK_MS :
                        (animationMode == ANIM_FADE)     ? FADE_TICK_MS     :
                        (animationMode == ANIM_SQUEEZE)  ? SQUEEZE_TICK_MS  :
                        (animationMode == ANIM_SNAKE)    ? SNAKE_TICK_MS    :
                        (animationMode == ANIM_FLICKER)  ? FLICKER_TICK_MS  : ANIM_TICK_MS;
  if (millis() - lastAnimTick < tickMs) return;
  lastAnimTick = millis();
  switch (animationMode) {
    case ANIM_MATRIX:   matrixTick();   break;
    case ANIM_TELETYPE: teletypeTick(); break;
    case ANIM_FADE:     fadeTick();     break;
    case ANIM_SQUEEZE:  squeezeTick();  break;
    case ANIM_SNAKE:    snakeTick();    break;
    case ANIM_FLICKER:  flickerTick();  break;
  }
}
