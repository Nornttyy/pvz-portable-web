// A real, tappable input over the native edit widget. Mobile keyboard focus
// must happen in the touch gesture, not in the engine's next animation frame.
(() => {
  const input = document.getElementById('pvz-soft-keyboard');
  const canvas = Module.canvas;
  const container = document.getElementById('canvas-container');
  const enabled = navigator.maxTouchPoints > 0 || window.matchMedia?.('(pointer: coarse)').matches;
  const bridge = Module.pvzTextInput = {
    enabled: Boolean(enabled), active: false, field: null, pending: null,
    keys: [], composing: false, applying: false,
    sync(field) {
      if (!this.enabled) return;
      this.field = field;
      if (!this.active || !field) return;
      position();
      if (this.composing || this.pending || this.applying) return;
      if (input.value !== field.text) input.value = field.text;
      if (input.selectionStart !== field.start || input.selectionEnd !== field.end)
        input.setSelectionRange(field.start, field.end);
    },
    start() {
      if (!this.enabled) return;
      if (!this.active) {
        this.active = true; this.pending = null; this.keys.length = 0;
        this.composing = false; this.applying = false;
      }
      input.dataset.nativeEdit = 'true';
      this.sync(this.field);
      // Do not focus here: native GotFocus is asynchronous on mobile.
      // The user's tap on this visible input opens the system keyboard.
    },
    stop() {
      this.active = false; this.pending = null; this.keys.length = 0;
      this.composing = false; this.applying = false;
      delete input.dataset.nativeEdit;
      input.blur();
      container.style.height = ''; container.style.top = '';
      Module.pvzResizeCanvas?.();
    },
    popKey() { return this.keys.shift() || 0; },
    hasEvents() { return Boolean(this.pending || this.keys.length); },
  };
  function position() {
    const field = bridge.field;
    if (!bridge.active || !field) return;
    if (window.visualViewport) {
      container.style.height = `${window.visualViewport.height}px`;
      container.style.top = `${window.visualViewport.offsetTop}px`;
      Module.pvzResizeCanvas?.();
    }
    const rect = canvas.getBoundingClientRect();
    if (!rect.width || !rect.height) return;
    const sx = rect.width / field.gameWidth, sy = rect.height / field.gameHeight;
    input.style.left = `${rect.left + field.x * sx}px`;
    input.style.top = `${rect.top + field.y * sy}px`;
    input.style.width = `${Math.max(40, field.width * sx)}px`;
    input.style.height = `${Math.max(24, field.height * sy)}px`;
    input.style.fontSize = `${Math.max(16, 20 * sy)}px`;
  }
  function queueEdit() {
    if (!bridge.active || bridge.composing || !bridge.field) return;
    bridge.pending = { owner: bridge.field.owner, text: input.value,
      start: input.selectionStart, end: input.selectionEnd };
  }
  input.addEventListener('input', event => {
    if (!bridge.enabled) return;
    event.stopPropagation();
    if (!event.isComposing) queueEdit();
  });
  input.addEventListener('beforeinput', event => {
    if (!bridge.active || bridge.composing || event.isComposing) return;
    if (event.inputType === 'insertLineBreak' || event.inputType === 'insertParagraph') {
      event.preventDefault(); event.stopPropagation(); queueEdit(); bridge.keys.push(13);
    }
  });
  input.addEventListener('compositionstart', () => { if (bridge.active) bridge.composing = true; });
  input.addEventListener('compositionend', () => {
    if (!bridge.active) return;
    bridge.composing = false; queueEdit();
  });
  input.addEventListener('select', () => {
    if (document.activeElement === input && bridge.field &&
      (input.value !== bridge.field.text || input.selectionStart !== bridge.field.start || input.selectionEnd !== bridge.field.end)) queueEdit();
  });
  // Keep SDL's document-level handlers from eating IME events or submitting
  // a second Enter. Normal browser editing handles deletion and selection.
  for (const type of ['keydown', 'keyup', 'keypress']) input.addEventListener(type, event => {
    if (!bridge.active) return;
    event.stopPropagation();
    if (type !== 'keydown' || event.isComposing || bridge.composing || event.keyCode === 229) return;
    if (event.key === 'Enter' || event.key === 'Escape' || event.key === 'Tab') {
      event.preventDefault(); queueEdit();
      bridge.keys.push(event.key === 'Enter' ? 13 : event.key === 'Escape' ? 27 : 9);
    }
  });
  for (const type of ['pointerdown', 'pointerup', 'touchstart', 'touchend', 'mousedown', 'mouseup', 'click']) input.addEventListener(type, event => {
    if (!bridge.active) return;
    event.stopPropagation();
    if (type === 'touchend' || type === 'pointerup') input.focus({preventScroll: true});
  });
  window.addEventListener('resize', position);
  window.visualViewport?.addEventListener('resize', position);
  window.visualViewport?.addEventListener('scroll', position);
  document.addEventListener('fullscreenchange', position);
})();
