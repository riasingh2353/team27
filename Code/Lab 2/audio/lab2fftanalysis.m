Fs = 9600;
for k = 1:128
    freqs(k) = k*Fs/256; %calculate values of frequency bins
end

fftdata = [ %PASTE OUTPUT OF teamalphafft.ino HERE!!!
]




hold on
for k = 1:length(fftdata(:,1))
    plot(freqs, fftdata(k,:)) 
end
xlabel('frequency (Hz)')
ylabel('magnitude (dB)')
