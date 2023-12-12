# Vibration Sensor

This is a vibration sensor to detect malfunction in equipment based on anomalies/shifting in frequency-domain data.

The sensor performs an accelerometer reading of $N$ samples over 1s, which is stored in FIFO. The sequence $x[n]$ for $n=0..N-1$ is mapped  $\mathbb{R}^N \rightarrow \mathbb{C}^N$ by DFT:
$$X(k) = \sum_{n=0}^{N-1} x[n] \cdot e^{-j \frac{2\pi}{N}kn}$$


The DFT is computed using the CMSIS-DSP mixed radix FFT algorithm.

The resulting array $X(k)$ has N complex values, which is not recommended to transfer over a low-rate network like 802.15.4, especially with multiple devices. 
For the scope of this project, the PSD of the frequency spectra, given by $S_{xx}(k) = |X(k)|^2$, was found to be largely contained in a few specific bins (with some minor spectral leakage in adjacent bins), and negligible values for all other bins.
<br>

To achieve reasonable compression, a noise floor is calculated:
$$\text{NF} = k \times \text{MAD}(|X(k)|^2)$$
where $$\text{MAD} = \frac{1}{n} \sum_{i=1}^{n} |x_i - m(X)|$$

such that 
```math
X(k) = \begin{cases}
    X(k) & \text{if } X(k) \geq \text{NF} \\
    0 & otherwise
\end{cases}
```
Then all zero-valued coefficients are not transmitted regularly.
