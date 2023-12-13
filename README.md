# Vibration Sensor

This is a vibration sensor to detect malfunction in equipment based on anomalies/shifting in frequency-domain data.

The sensor performs an accelerometer reading of $N$ samples over 1s, which is stored in FIFO. The temporal sequence $x[n]$ for $n=0..N-1$ is mapped  $\mathbb{R}^N \rightarrow \mathbb{C}^N$ by DFT:
$$X(k) \triangleq \sum_{n=0}^{N-1} x[n] \cdot e^{-j \frac{2\pi}{N}kn}$$


The DFT is computed using the CMSIS-DSP mixed radix FFT algorithm.

The resulting array $X(k)$ has $N$ complex values; it is nontrivial to transmit this sequence over a low-rate network like 802.15.4, especially with multiple devices present.
It was observed that most of the useful data is found within specific discrete frequency bins, with the other values being negligibly small.<br>
More formally, the PSD of the frequency spectra, given by $S_{xx}(k) = |X(k)|^2$, was found to be largely contained in a few specific bins (with some minor spectral leakage in adjacent bins), and negligible values for all other bins.<br>
Since there is not a known $k$ for which $X(k)$ is only noise,
it suffices to define a set $M = \lbrace m \mid 0 \leq m < N \enspace \land \enspace \lvert X(m) - 0 \rvert \leq \varepsilon \rbrace$ where $\varepsilon$ is defined subject to maximizing $\lvert M \rvert$ , and minimizing $\sum_{m \in {M}} |X(m)|^2$. <br><br>
Then $\frac{N}{|M|}$ approximates the compression ratio.
<br>



An estimation of the noise floor is derived for thresholding:
$$\text{NF} = c \cdot \text{MAD}(|X(k)|^2),  \quad  c \in \mathbb{R}^+ $$
where $$\text{MAD} = \frac{1}{n} \sum_{i=1}^{n} |x_i - m(X)|$$

such that 
```math
X(k) = \begin{cases}
    X(k) & \text{if } 0 \leq k \leq N-1 \quad \text{and } X(k) \geq \text{NF} \\
    0 & \text{otherwise}
\end{cases}
```
It follows that only the nonzero values are transmitted (and the NF).
In this case, N = 4000, so the straightforward $O(n)$ algorithm to calculate average and $O(n)$ auxillary space (the transmit buffer) is used.


