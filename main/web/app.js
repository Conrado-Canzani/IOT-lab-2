const sliderR = document.getElementById('sliderR');
const sliderG = document.getElementById('sliderG');
const sliderB = document.getElementById('sliderB');
const valR    = document.getElementById('valR');
const valG    = document.getElementById('valG');
const valB    = document.getElementById('valB');
const preview = document.getElementById('ledPreview');
const status  = document.getElementById('ledStatus');
const feedback = document.getElementById('feedback');

let userInteracting = false;
let interactionTimer = null;

function updatePreview() {
    const r = parseInt(sliderR.value);
    const g = parseInt(sliderG.value);
    const b = parseInt(sliderB.value);

    valR.textContent = r;
    valG.textContent = g;
    valB.textContent = b;

    const isOff = r === 0 && g === 0 && b === 0;
    preview.style.background = isOff ? '#000' : `rgb(${r}, ${g}, ${b})`;
    preview.style.boxShadow  = isOff ? 'none'
        : `0 0 24px 6px rgba(${r}, ${g}, ${b}, 0.5)`;
    status.textContent = isOff ? 'Apagado' : `rgb(${r}, ${g}, ${b})`;
}

function onSliderInteraction() {
    userInteracting = true;
    clearTimeout(interactionTimer);
    // Reanuda el polling 2 segundos después de que el usuario deje de tocar
    interactionTimer = setTimeout(() => {
        userInteracting = false;
    }, 2000);
    updatePreview();
}

sliderR.addEventListener('input', onSliderInteraction);
sliderG.addEventListener('input', onSliderInteraction);
sliderB.addEventListener('input', onSliderInteraction);

function showFeedback(msg, type) {
    feedback.textContent = msg;
    feedback.className = 'feedback ' + type;
    setTimeout(() => {
        feedback.textContent = '';
        feedback.className = 'feedback';
    }, 2500);
}

async function applyColor() {
    const r = parseInt(sliderR.value);
    const g = parseInt(sliderG.value);
    const b = parseInt(sliderB.value);

    try {
        const resp = await fetch('/led', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ r, g, b })
        });
        if (resp.ok) {
            showFeedback('Color aplicado correctamente', 'ok');
            userInteracting = false; // permite polling inmediato
        } else {
            showFeedback('Error: ' + resp.status, 'err');
        }
    } catch (e) {
        showFeedback('Sin respuesta del dispositivo', 'err');
    }
}

async function turnOff() {
    sliderR.value = 0;
    sliderG.value = 0;
    sliderB.value = 0;
    updatePreview();
    await applyColor();
}

async function loadCurrentState() {
    try {
        const resp = await fetch('/led');
        if (!resp.ok) return;
        const data = await resp.json();
        sliderR.value = data.r ?? 0;
        sliderG.value = data.g ?? 0;
        sliderB.value = data.b ?? 0;
        updatePreview();
    } catch (e) {
        updatePreview();
    }
}

// Polling cada 500ms, solo si el usuario no está interactuando
setInterval(async () => {
    if (userInteracting) return;
    try {
        const resp = await fetch('/led');
        if (!resp.ok) return;
        const data = await resp.json();
        sliderR.value = data.r ?? 0;
        sliderG.value = data.g ?? 0;
        sliderB.value = data.b ?? 0;
        updatePreview();
    } catch (e) {}
}, 500);

loadCurrentState();