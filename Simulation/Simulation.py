import numpy as np
import matplotlib.pyplot as plt

N = 128 #number of samples
fe = 256 #sampling frequency
df = fe/N

n = np.arange(0, N)
t = n / fe #time vector

#x: np.ndarray = 5*np.sin(2*np.pi*40*t) + 1*np.sin(2*np.pi*35*t) + 2*np.sin(2*np.pi*120*t)
x: np.ndarray = 1*np.sin(2*np.pi*35*t)+1*np.sin(2*np.pi*40*t)+ 2*np.sin(2*np.pi*1000*t)

X: np.ndarray = np.fft.fft(x)

frequencies = np.fft.fftfreq(N, d=1/fe)
mask = (frequencies >= 35) & (frequencies <= 50)

X_filtered = np.zeros_like(X)
X_filtered[mask] = X[mask]

fig, ax = plt.subplots()
ax.set_xscale('log')  # Définit l'échelle logarithmique pour l'axe des x
ax.stem(
    frequencies[:N//2],
    20 * np.log10(np.abs(X_filtered[:N//2])), 
    basefmt=" "  # Supprime la ligne horizontale de base
)
ax.set(
    title=f"Amplitude des fréquences entre 35 Hz et 50 Hz",
    ylabel="|X| (dB)",
    xlabel="Fréquence (Hz)",
    xlim=(10, 100)
)
plt.savefig("Amplitudes.png")

# Moyenne de la puissance du signal
moyenneSpectre = np.mean(np.abs(X_filtered)**2)

print(f"Moyenne du spectre filtré = {moyenneSpectre}")
#plt.show()

print("Breakpoint")

