// Cache-busting: bump this, the ?v=N on the <script> tag in index.html,
// and the wasm loader's own file version below, every time this file or
// the compiled interpreter changes -- otherwise GitHub Pages/the browser
// can keep serving an old cached copy (bit us once already on the
// rust_spreadsheet demo, doing it right from the start here).
import createInterpreterModule from "./pkg/interpreter.js?v=1";

const REG_LAYOUT = [
  ["r0", 0], ["r1", 1], ["r2", 2], ["r3", 3],
  ["r4", 4], ["r5", 5], ["r6", 6], ["r7", 7],
  ["r8", 8], ["r9", 9], ["r10", 10], ["r11", 11],
  ["r12", 12], ["r13", 13], ["sp (r14)", 14], ["ra (r15)", 15],
];

const EXAMPLES = {
  arith: `.main:
    mov r0, 7
    mov r1, 6
    add r2, r0, r1
    mul r3, r0, r1
    .print r0, r1, r2, r3
`,
  loop: `.main:
    mov r0, 0        @ counter
    mov r1, 5        @ limit (exclusive)
    mov r2, 0        @ running sum

loop:
    cmp r0, r1
    beq done
    add r2, r2, r0
    add r0, r0, 1
    b loop

done:
    .print r2
`,
  call: `.main:
    mov r0, 10
    mov r1, 20
    call add_two
    .print r2
    b end

add_two:
    add r2, r0, r1
    ret

end:
`,
  memory: `.main:
    mov r0, 5
    st r0, 0[r14]
    ld r1, 0[r14]
    .print r0, r1
`,
  encode: `.main:
    .encode add r0, r1, r2
    .encode mov r3, 5
    .encode st r0, 0[sp]
`,
  error: `.main:
    foo r0, r1, r2
`,
};

const sourceEl = document.getElementById("source");
const outputEl = document.getElementById("output");
const regTableEl = document.getElementById("regTable");
const flagEEl = document.getElementById("flagE");
const flagGTEl = document.getElementById("flagGT");
const memAddrEl = document.getElementById("memAddr");
const memValEl = document.getElementById("memVal");

let Module;
let runProgram, getReg, getFlag, getMem;
let pendingOutput = [];

function appendOutput(text, isErr) {
  pendingOutput.push({ text, isErr: !!isErr });
}

function flushOutput() {
  if (pendingOutput.length === 0) {
    outputEl.textContent = "(no output)";
    return;
  }
  outputEl.innerHTML = pendingOutput
    .map((l) => (l.isErr ? `<span class="err">${escapeHtml(l.text)}</span>` : escapeHtml(l.text)))
    .join("\n");
}

function escapeHtml(s) {
  return s.replace(/[&<>]/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;" }[c]));
}

function buildRegTable() {
  let html = "";
  for (let row = 0; row < 4; row++) {
    html += "<tr>";
    for (let col = 0; col < 4; col++) {
      const [name, idx] = REG_LAYOUT[row * 4 + col];
      html += `<td class="name">${name}</td><td class="val" id="reg-${idx}">0</td>`;
    }
    html += "</tr>";
  }
  regTableEl.innerHTML = html;
}

function renderRegisters() {
  for (const [, idx] of REG_LAYOUT) {
    document.getElementById(`reg-${idx}`).textContent = getReg(idx);
  }
  const e = getFlag(0);
  const gt = getFlag(1);
  flagEEl.textContent = e;
  flagEEl.className = e ? "on" : "off";
  flagGTEl.textContent = gt;
  flagGTEl.className = gt ? "on" : "off";
  updateMemPeek();
}

function updateMemPeek() {
  const addr = parseInt(memAddrEl.value, 10) || 0;
  memValEl.textContent = getMem(addr);
}

function run() {
  pendingOutput = [];
  try {
    runProgram(sourceEl.value);
  } catch (e) {
    // run_program() calls C's exit() on any parse/runtime error (invalid
    // instruction, missing .main label, etc). Emscripten turns that into
    // a thrown JS exception to unwind out of the WASM call -- the actual
    // human-readable reason was already printed (captured into
    // pendingOutput below) before exit() ran, so there's nothing more to
    // show here; the WASM instance itself is still alive and reusable.
  }
  flushOutput();
  renderRegisters();
}

document.getElementById("runBtn").addEventListener("click", run);
document.getElementById("clearOutputBtn").addEventListener("click", () => {
  pendingOutput = [];
  flushOutput();
});
memAddrEl.addEventListener("input", updateMemPeek);

document.querySelectorAll(".example").forEach((btn) => {
  btn.addEventListener("click", () => {
    sourceEl.value = EXAMPLES[btn.dataset.example] || "";
  });
});

async function main() {
  Module = await createInterpreterModule({
    print: (text) => appendOutput(text, false),
    printErr: (text) => appendOutput(text, true),
  });
  runProgram = Module.cwrap("run_program", null, ["string"]);
  getReg = Module.cwrap("get_reg", "number", ["number"]);
  getFlag = Module.cwrap("get_flag", "number", ["number"]);
  getMem = Module.cwrap("get_mem", "number", ["number"]);

  buildRegTable();
  sourceEl.value = EXAMPLES.arith;
  renderRegisters();
  flushOutput();

  document.getElementById("loading").style.display = "none";
  document.getElementById("app").style.display = "block";
}

main();
