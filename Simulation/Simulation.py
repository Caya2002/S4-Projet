import numpy as np
import matplotlib.pyplot as plt

N = 128 #number of samples
fe = 640 #sampling frequency
df = fe/N
f = 35

n = np.arange(0, N)
t = n / fe #time vector

scale = 1/32

#x: np.ndarray = 5*np.sin(2*np.pi*40*t) + 1*np.sin(2*np.pi*35*t) + 2*np.sin(2*np.pi*120*t)
#x = 10*np.sin(2*np.pi*f*t)
#x = (128*np.sin(2*np.pi*scale*f*t)); #partie reelle  = sin(), partie imag = 0 (<<16)
#x = (128*np.sin(2*np.pi*scale*f*t)).astype(np.int16) #partie reelle  = sin(), partie imag = 0 (<<16)
x = (1*np.sin(2*np.pi*f*t)).astype(np.int16)
#x = ((10*np.sin(2*np.pi*f*t) + 10*np.sin(2*np.pi*100*t))).astype(np.int16)

plt.figure()
plt.plot(t, x)
plt.title(f"Signal temporel (f = {f} Hz)")
plt.xlabel("Temps (s)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.savefig(f"Signal_temporel_f_{f}.png")  # Sauvegarde du graphique

X: np.ndarray = np.fft.fft(x)

frequencies = np.fft.fftfreq(N, d=1/fe)
mask = (frequencies >= 35) & (frequencies <= 50)

plt.figure()
plt.plot(n, 20*np.log10(np.abs(X)))
plt.title(f"Transformée de Fourier (f = {f} Hz)")
plt.xlabel("Échantillons [k])")
plt.ylabel("|X| [dB]")
plt.grid(True)
plt.savefig(f"FFT_f_{f}.png")  # Sauvegarde du graphique

powerSpectrum = np.zeros_like(n)
#powerSpectrum[mask] = (X[mask].real * X[mask].real) + (X[mask].imag * X[mask].imag)
powerSpectrum = (X.real * X.real) + (X.imag * X.imag)
powerSpectrum_filtered = np.zeros_like(n)
powerSpectrum_filtered[mask] = powerSpectrum[mask]

fig, ax = plt.subplots()
ax.set_xscale('log')
ax.stem(
    frequencies[:N//2],
    20 * np.log10(powerSpectrum[:N//2]+1),
    #20 * np.log10(powerSpectrum_filtered[:N//2]+1),
    basefmt=" "  # Supprime la ligne horizontale de base
)
ax.set(
    title=f"Amplitude des fréquences entre 35 Hz et 50 Hz (f = {f} Hz)",
    ylabel="|X| (dB)",
    xlabel="Fréquence (Hz)",
    xlim=(10, 100)
)
ax.grid(which='both', linestyle='--', linewidth=0.5)
plt.savefig(f"Amplitudes_f_{f}.png")

# Moyenne de la puissance du signal
#moyenneSpectre = np.mean(X_filtered)

dF = fe / N
total_concentration = 0
MIN_BIN = int(35 / dF)
MAX_BIN = int(50 / dF)

total_concentration = 0
for bin in range(MIN_BIN, MAX_BIN):
    total_concentration += powerSpectrum[bin]

moyenneSpectre = total_concentration / (MAX_BIN - MIN_BIN)

print(f"Moyenne = {moyenneSpectre:.2f} avec {f} Hz")
#plt.show()

print("Breakpoint")

