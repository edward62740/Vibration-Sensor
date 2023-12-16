# Vibration Sensor [in progress]

The goal of this project is to develop a battery powered sensor that can be used to perform predictive maintenance of mechanical equipment through frequency-domain analysis.
In general, such analysis is performed using traditional algorithms or ML (on the edge or remotely). This project provides basic on-device neural network using the built-in MVP accelerator, as well as transmitting the processed data to a server for analysis.


## Theory of Operation
The processed data is acquired as follows:<br>
The sensor performs an accelerometer [^1] reading of $N$ samples over 1s, which is stored in FIFO. The real-valued temporal sequence $x[n]$ for $n=0..N-1$ is mapped  $\mathbb{R}^N \rightarrow \mathbb{C}^N$ by DFT:
$$X(k) \triangleq \sum_{n=0}^{N-1} x[n] \cdot e^{-j \frac{2\pi}{N}kn}$$


The DFT is computed using the CMSIS-DSP mixed radix FFT algorithm.


For real-valued sequences $X(k) = \overline{X(N - k)}$, so $N$ samples in time domain can be sufficiently represented by $\lfloor N/2 \rfloor -1$ complex values.

The resulting array $X(k)$ still has $N/2$ complex values; it is nontrivial to transmit this sequence over a low-rate network like 802.15.4, especially with multiple devices present.
It was observed that most of the useful data is found within specific discrete frequency bins, with the other values being negligibly small.<br>
More formally, the PSD of the frequency spectra, given by $S_{xx}(k) = |X(k)|^2$, was found to be largely contained in a few specific bins (with some minor spectral leakage in adjacent bins), and negligible values for all other bins.<br>
Since there is not a known $k$ for which $X(k)$ is only noise, then the optimization problem:<br>
given $M = \lbrace m \mid 0 \leq m < N \enspace \land \enspace \lvert X(m) - 0 \rvert \leq \varepsilon \rbrace$,<br>
$\varepsilon \in \mathbb{R}$ is defined subject to maximizing  $\lvert M \rvert$ , and minimizing  $\sum_{m \in {M}} |X(m)|^2$. <br><br>
For which $\frac{N}{|M|}$ approximates the compression ratio.

<br>



It is sufficient to derive an estimation of the noise floor for thresholding:
$$\text{NF} = c \cdot \text{MAD}(|X(k)|^2),  \quad  c \in \mathbb{R}^+ $$
where $$\text{MAD} = \frac{1}{n} \sum_{i=1}^{n} |x_i - m(X)|$$

such that 
```math
X(k) = \begin{cases}
    X(k) & \text{if } 0 \leq k \leq N-1 \quad \text{and } X(k) \geq \text{NF} \\
    null & \text{otherwise}
\end{cases}
```
It follows that only the non-null values are transmitted (and the NF as a single value).

In this case, N = 4096, so the straightforward $O(n)$ algorithm to calculate average and $O(n)$ auxillary space (the transmit buffer) is used.<br>
The above algorithm can be repeated for each axis (x,y,z) as needed.

[^1]: The accelerometer used (IIS3DWB) specifies a noise density for frequencies 0.1-6.3kHz.
## ML

## Communication

## Battery Life and Performance
