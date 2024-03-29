# Effetto echo realizzato con Raspberry Pi Pico 

### _Un esercizio per illustrare alcuni concetti dell'elaborazione audio real-time lineare e una sua modellizzazione_

<p>

</p>

## File "simple_echo.ino"

Questo semplice dispositivo realizzato con un Raspberry Pi Pico ed una coppia di DAC I2S MAX98357A, è un effetto echo stereo (anche detto delay) con campionamento 12bit - 40Ksps (samples per second); non include alcuna regolazione esterna né filtri anti aliasing agli ingressi, perchè lo scopo è puramente didattico.

Al Pi Pico è demandato il ruolo di effettuare la lettura/campionamento dei segnali in ingresso (left - right) utilizzando il convertitore ADC 12bit interno alla board, quindi eseguire le elaborazioni necessarie (scrivere nelle code FIFO i sample letti, leggere sample dalle code FIFO, effettuare moltiplicazioni e somme), infine inviare un flusso audio digitale 16bit - 40Ksps a ciascuno dei due DAC, utilizzando connessioni fisiche a 3 fili con protocollo I2S.

A loro volta i DAC effettuano una conversione da digitale a PWM, con frequenza 300Khz, utilizzando in uscita un circuito a bassa impedenza d'uscita ed elevata corrente, in grado quindi di pilotare direttamente gli altoparlanti 4-8Ω; le caratteristiche di impedenza e risposta elettromeccanica degli altoparlanti abbattono e rendono inudibili le componenti audio del flusso PWM superiori 15-20kHz.

Nel video seguente una breve registrazione dell'effetto; all'ingresso è collegato un generatore di funzioni con sweep regolato manualmente:

https://github.com/fablabromagna-org/sound-synth-midi/assets/41198542/2fde0d05-7ad2-4d5e-9d04-8c904a834ee6


### Schema a blocchi

Lo schema ingresso-uscita seguente illustra l'articolazione dell'effetto echo nei suoi componenti: il delay ed i blocchi sommatore e moltiplicatore , l'interconnessione tra gli stessi e la direzione del flusso di elaborazione.

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/delay_0.jpg")
</p>

Lo schema nasce da una semplice intuizione: per esperienza comune un suono con eco è dato dalla somma (in aria) del suono origine, dello stesso suono riflesso (che giunge con un certo ritardo all'origine), e delle ulteriori riflessioni delle riflessioni.

Nel passaggio dallo schema a blocchi al codice occorre tener presente che il segnale trattato non è continuo ma campionato; non a tempo continuo ma a tempo discreto. Questo significa che occorre elaborare singoli campioni e fornire in uscita, analogamente, singoli campioni.

Nell'esercizio, il codice ha la totale responsabilità del rispetto del timing. La funzione "repeating_timer_callback", che viene chiamata via interrupt ogni 25us (tempo di campionamento), svolge la funzione di leggere (campionare) i due segnali in ingresso (utilizzando i convertitori AD interni alla scheda Raspberry Pi Pico), effettuare le elaborazioni audio richieste, infine inviare due campioni (sample) ai due convertitori DA esterni. 25us rappresenta il tempo concesso per effettuare l'elaborazione audio desiderata, e corrisponde ad una frequenza di campionamento pari a 40ksps.

In termini di codice, lo schema a blocchi si traduce in una singola funzione lineare; infatti se:
- $D$ è il valore del ritardo (come numero di sample) introdotto dal blocco delay
- $x(n)$ il sample in ingresso all'istante n
- $y(n)$ il sample in uscita all'istante n

allora il sample che possiamo leggere in uscita dal delay è $y(n-D)$:

<p align="left">
<img width="400" src="/pi_pico_echo_stereo/media/z_13.jpg")
</p>

All'uscita del blocco sommatore risulta quindi:

(1) $y(n) = Cx(n) + Ky(n-D)$

La presenza dei parametri $C$ e $K$ è prima di tutto dovuta al fatto che la lunghezza di parola è finita (16bit), ed essi aiutano a prevenire fenomeni di saturazione numerica. Il parametro $K$ ha una ulteriore importante ricaduta, perchè da esso dipende la **stabilità** dell'algoritmo: per valori $K>1$ il calcolo diverge rapidamente in presenza del più piccolo e breve segnale in ingresso. Tratteremo il tema della stabilità nelle sezioni successive introducendo ulteriori semplici strumenti di analisi.

Il blocco delay è realizzato con un array di dimensione almeno pari a $D+1$; nel codice si usano due array, uno per canale, di dimensione 14000, in grado di memorizzare 14000 sample, pari a 14000\*25us=350ms; il valore massimo per D è quindi 13999\*25us.


### Modellizzazione

L'algoritmo riceve in input una successione di valori (sample) misurati dell'ingresso e fornisce in output un'altra successione di valori:

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/z_0.jpg")
</p>

Indichiamo con $x(n)$ la sequenza di sample in ingresso, $y(n)$ la sequenza di sample in uscita, con $n = 0, 1, 2,$... In quale relazione stanno le due sequenze? Quale la risposta in frequenza? Cosa accade modificando i parametri C e D? Per rispondere senza ricorrere a modelli matematici occorre costruire materialmente il dispositivo, scriverne e compilarne il codice, realizzare un banco di prova con generatore di funzioni e oscilloscopio per la visualizzazione dei due segnali. In alternativa si crea un _modello matematico_, e lo si studia con simulazioni automatiche. Per le sequenze a tempo discreto, come sono le successioni regolari di campioni, è stato definito da _Laplace_ un metodo modello matematico che agevola lo studio di sistemi di elaborazione lineari (cioè costituiti da moltiplicatori per costanti, sommatori e ritardi) a tempo disceto: la _trasformata Z_.

![Laplace](https://upload.wikimedia.org/wikipedia/commons/thumb/e/e3/Pierre-Simon_Laplace.jpg/260px-Pierre-Simon_Laplace.jpg)

<sup>_Ritratto di Pierre-Simon Laplace (1749-1827)_</sup>


#### Trasformiamo una serie di campioni in una funzione
Per descrivere una successione di campioni in termini matematici, torna comodo esprimerla come una funzione; definiamo una particolare funzione discreta detta _impulso unitario_ $δ(n)$, così definita:

(2) **$δ(n)$ vale $1$ per $n=0$ ; vale $0$ per ogni altro valore di $n$**

Se vogliamo spostare il campione unitario nella posizione $k$ la funzione diventa $δ(n-k)$; infatti:
     
(3) **$δ(n-k)$ vale $1$ per $n-k=0$ ossia per $n=k$ ; vale $0$ per ogni altro valore di $n$**

<p align="left">
<img width="400" src="/pi_pico_echo_stereo/media/z_2.jpg")
</p>

Utilizzando la funzione impulso unitario $δ(n-k)$ possiamo descrivere una qualsiasi sequenza di campioni in forma di funzione; data infatti la sequenza di campioni:

(4) $x(0), x(1), x(2),$ ...

possiamo rappresentarla come funzione $x(n)$, costituita da una combinazione lineare di impulsi unitari via via ritardati:

(5) $x(n) = x(0)δ(n) + x(1)δ(n-1) + x(2)δ(n-2) +$ ...

#### Enunciamo la traformata Z ed applichiamola alla serie di campioni
La trasformata Z altro non è che una semplice applicazione sulla successione $x(n)$. Definiamo $X(z)$ _trasformata Z di $x(n)$_ la funzione che si ottiene sostituendo $δ(n-k)$ con $z^{-k}$:

(6) $X(z) = x(0)z^0 + x(1)z^{-1} + x(2)z^{-2} +$ ...

e ricordando che per qualsiasi valore $z$ si ha $z^0 = 1$:

(7) $X(z) = x(0) + x(1)z^{-1} + x(2)z^{-2} +$ ...

Si noti che $Δ(z)$, trasformata Z di $δ(n)$, vale semplicemente 1; infatti in questo caso:

(8) $Δ(z) = 1 + 0z^{-1} + 0z^{-2} +$ ... $= 1$

Infine, se consideriamo la generica successione $r(n)$ ottenuta ritardando la serie $y(n)$ di $D$ campioni:

(9) $r(n) = 0δ(n) + 0δ(n-1) + 0δ(n-2) +$ ... $+ x(0)δ(n-D) + x(1)δ(n-D-1) + x(2)δ(n-D-2) +$ ... $= x(0)δ(n-D) + x(1)δ(n-D-1) + x(2)δ(n-D-2) +$ ...

Trasformando $r(n)$, otteniamo:

(10) $R(z) = x(0)z^{n-D} + x(1)z^{n-D-1} + x(2)z^{n-D-2} +$ ... $= z^{-D}(x(0)δ(n) + x(1)δ(n-1) + x(2)δ(n-2) +$ ... $) = z^{-D}X(Z)$

Cioé: la trasformata Z di una successione $x(n)$ ritardata di $D$ campioni si ottiene moltiplicando $X(z)$ per $z^{-D}$.

Infine, altra importante proprietà della trasformata Z: date due successioni, $x(n)$ ed $y(n)$ e la successione somma $s(n)=x(n)+y(n)$, la trasformata Z di $s(n)$ vale:

(11) $S(z) = X(z) + Y(z)$


#### Calcoliamo la funzione di trasferimento Z del nostro echo
Abbiamo visto la relazione ingresso uscita dell'algoritmo per l'echo:

(12) $y(n) = Cx(n) + Ky(n-D)$

che rappresenta un'uguaglianza tra due successioni; applicando la trasformata Z ad entrambi i membri otteniamo:

(13) $Y(z) = CX(z) + Kz^{-D}Y(z)$

da cui:

(14) $Y(z) = X(z) * C/(1 - Kz^{-D})$

Definiamo il secondo termine di questa moltiplicazione **funzione di trasferimento H(z)** del nostro echo:

(15) $H(z) = C/(1 - Kz^{-D}) = Cz^D/(z^D - K)$

Infine scriviamo:

(16) $Y(z) = X(z)H(z)$

<p align="left">
<img width="300" src="/pi_pico_echo_stereo/media/z_3.jpg")
</p>

Attraverso questa serie di passaggi abbiamo trasformato la relazione che lega ingresso e uscita in modo **implicito**, in una nuova relazione ingresso-uscita più astratta ma **esplicita**.



### Cosa ne facciamo di H(z)? Condizioni per la stabilità
La conoscenza della funzione di trasferimento dell'echo consente per prima cosa di studiarne la stabilità. Le funzioni di trasferimento ricavate da algoritmi lineari sono **funzioni razionale fratte** in $z$, cioè esprimibili con un numeratore $A(z)$ ed un denominatore $B(z)$ che sono polinomi in $z$:

(17) $H(z) = A(z)/B(z)$

Siano $A(z)$ e $B(z)$ polinomi di grado rispettivamente $N$ e ed $M$:

(18) $A(z) = a_Nz^N + a_{N-1}z^{N-1} + a_{N-2}z^{N-2} +$ ..... $+ a_0$
(19) $B(z) = b_Mz^M + b_{M-1}z^{M-1} + b_{M-2}z^{M-2} +$ ..... $+ b_0$

Siano $r_0, r_1, r_2$ ... $r_{N-1}$ le $N$ radici del polinomio $A(z)$; possiamo scrivere:

(20) $A(z) = a_N(z - r_0)(z - r_1)(z - r_2)$ ... $(z - r_{N-1})$

Analogamente, siano $p_0, p_1, p_2$ ..... $p_{M-1}$ le $M$ radici del polinomio $B(z)$; scriviamo:

(21) $B(z) = b_M(z - p_0)(z - p_1)(z - p_2)$ ... $(z - p_{M-1})$

Definiamo **_poli_** della funzione $H(z)$ gli $M$ valori $p_0, p_1, p_2$ ... $p_{M-1}$; si dimostra che:

**dato un sistema descritto dalla funzione di trasferimento discreta $H(z)$, se $H(z)$ presenta almeno un polo di valore assoluto maggiore o uguale ad 1, allora il sistema ha un comportamento _instabile_, ossia la sua uscita diverge o oscilla indipendentemente dall'ingresso; diversamente, il sistema ha un comportamento _stabile_**.

Riprendiamo ora la funzione di trasferimento $H(z)$ del nostro echo:

(22) $H(z) = Cz^D/(z^D - K)$

i cui poli sono i valori di $z$ per cui di $(z^D - K)$ si annulla, cioè per cui vale:

(23) $z^D = K$

Si tratta di una particolare equazione di grado $D$ in $z$ (per approfondimenti: https://www.unife.it/ing/informazione/analisi-matematica-Ib/lezioni-ed-esercizi/lezione-4-radici-n-esime-in-campo-complesso), le cui $D$ radici hanno lo stesso modulo $\sqrt[D]{|K|}$, che è un numero minore di 1 solo se e solo se $|K|<1$: ciò significa che il nostro echo è stabile se e solo se $|K|<1$.


### Cosa ne facciamo di H(z)? Studio della risposta in frequenza
Utilizzando lo strumento di calcolo automatico online Mathworks (https://matlab.mathworks.com/) disponibile gratuitamente per un uso limitato a max 20h/mese, possiamo visualizzare la _risposta in frequenza_ del nostro echo. Nell'ambiente Mathworks, questa la descrizione della funzione di trasferimento $H(z)$

```
C = 0.5;
K = -0.8;
% D = 30;

% la funzione di trasferimento razionale è descritta nella forma b(z^-1)/c(z^-1)
% dove il numeratore b = b0 + b1z^-1 + b2z^-2 + b3z^-3 .....
% e il denominatore a = a0 + a1z^-1 + a2z^-2 + a3z^-3 .....

% a, b vengono descritti tramite vettori, in cui si inseriscono i soli coefficienti b_k e a_k
b = C;
a = [1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 - K];
```

Le successive istruzioni consentono la visualizzazione della risposta in frequenza:

```
% Frequency response
[h,w] = freqz(b,a,1000); % 1000 punti di valutazione

% Il passaggio al dominio delle frequenze avviene con la sostituzione seguente:
% z --> e^(jῶ)
%
% dove ῶ è la pulsazione normalizzata al tempo di campionamento T; detta F la frequenza di campionamento:
% ῶ = 2π/(t/T) = 2πf/F
% 
% quindi anche:
% f = (ῶF)/2π
% 
% i valori estremi sono:
% ῶ = 0 <--> f = 0
% ῶ = π <--> 2π*f/F = π <--> f = F/2 che rappresenta la frequenza di Nyquist (massima campionabile senza perdita di informazione)

plot((w*40)/(2*pi), 20*log10(abs(h))); % 40 corrisponde alla frequenza di campionamento scalata 40ksps/1000
ax = gca;
ax.XScale = 'log';
ax.YLim = [-20 20];
ax.XTick = 0:5:20; % primo elemento: incremento : ultimo elemento (primo ed ultimo entro il dominio del grafico)
xlabel('kHz');
ylabel('Magnitude (dB)');
```


Quindi, col set di valori:

$C = 0.5$, $K = 0.8$, $D = 10$

risulta il seguente andamento della risposta in frequenza:

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_4.jpg")
</p>

Aumentando il valore del ritardo $D$ introdotto dal delay, la risposta in frequenza diventa via via più movimentata; se ad esempio scegliamo $D = 30$:

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_5.jpg")
</p>


Possiamo poi sperimentare cosa accade se $K<0$; se quindi:

$C = 0.5$, $K = -0.8$, $D = 30$

otteniammo la seguente risposta in frequenza

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_6.jpg")
</p>

Si noti come l'inversione di fase dell'uscita (dovuta a $K<0$) riduca in modo sensibile tutta la parte bassa dello spettro, fino alla frequenza $F_0$ per cui il ritardo introdotto dal delay, pari a $30*25us=750us$, corrisponde alla metà del periodo $T_0$: il segnale fornito dal delay risulta "in fase" rispetto all'ingresso. Calcoliamo questa prima frequenza:

(24) &T_0/2=750us$

da cui:

(25) $F_0 = 1/T_0 \simeq 666Hz$.


### Cosa ne facciamo di H(z)? Studio della risposta all'impulso
Se l'analisi nel dominio delle frequenze fornisce informazioni sulla timbrica, l'analisi nel dominio dei tempi, in cui si confronta il segnale di ingresso col segnale in uscita, fornisce informazioni forse più utili sull'echo, trattandosi di un effetto cui si richiede fondamentalmente la moltiplicazione nel tempo di singoli eventi. Se:

$C = 0.5$, $K = 0.8$, $D = 30$

utilizzando le istruzioni seguenti:

```
C = 0.5;
K = 0.8;
D = 30;

% funzione di trasferimento
syms z
H1 = (C*z^D)/(z^D-K);

% ingresso
in = 1; % impulso unitario

% risposta nel dominio dei tempi
out=iztrans(H1*in);
Serie = subs(out,[sym("n")],0:300);
p=plot(Serie);
ax = gca;
ax.XScale = 'linear';
ax.YLim = [-1 1];
ax.XTick = 0:50:300; % primo elemento: incremento : ultimo elemento (primo ed ultimo entro il dominio del grafico)
xlabel('n')
ylabel('value')
```

possiamo visualizzare la risposta all'impulso, come una successione di impulsi a distanza di 30 campioni:

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_7.jpg")
</p>


Con $K = - 0.8$ otteniamo la seguente risposta, dove ogni successivo impulso viene invertito di valore:

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_8.jpg")
</p>


### Cosa ne facciamo di H(z)? Studio della risposta ad una sinusoide

Vediamo ora come calcolare l'uscita dell'echo applicando in ingresso un segnale sinusoidale. Partiamo dalla funzione $sin(ωt)$ con $ω=2\pi f$, dove $f$ è la frequenza; poi, visto che ci interessa inviare il segnale sinusoidale a partire da $t=0$, azzeriamo la sinusoide per $t<0$ moltiplicandola per una funzione detta **gradino unitario** $u(t)$ così definita:

(26) **$u(t)$ vale $0$ per $t<0$ ; vale $1$ per $t>=0$**

Il nostro ingresso di prova, nel tempo continuo, è quindi:

(27) $x(t) = u(t)*sin(2\pi ft)$

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_16.jpg")
</p>

Nel campionamento, che ricordiamo avviene con periodo $T_0$ e frequenza $F_0 = 1/T_0$, leggiamo campioni del segnale d'ingresso agli istanti $0$, $T_0$, $2T_0$, ... Quindi a $t$ sostituiamo $nT_0$ ossia $n/F_0$; analogamente ad $u(t)$ sostituiamo $u(nT_0)$ che possiamo anche scrivere $u(n)$ (il valore della funzione campionata vale comunque $1$)

Il nostro ingresso di prova, nel tempo discreto, è quindi:

(28) $x(n) = u(n)sin(2\pi nf/F_0)$

Poi, ponendo:

(29) $ω_0 = 2\pi f/F_0$

possiamo scrivere:

(30) $x(n) = u(n)sin(ω_0n)$


<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_14.jpg")
</p>


Su https://it.wikipedia.org/wiki/Trasformata_zeta troviamo la relativa trasformata Z tra le trasformate "notevoli":

<p align="left">
<img width="300" src="/pi_pico_echo_stereo/media/z_9.jpg")
</p>

Vediamo finalmente l'andamento dell'uscita per, ad esempio, $f = 5000Hz$; su Mathworks, usando il codice seguente:

```
omega0 = 2*pi*5000/40000; % 2pi*f/F
syms z

H1 = (C*z^D)/(z^D-K);
in = z*sin(omega0)/(z^2 - 2*z*cos(omega0) +1);

out=iztrans(H1*in);
Serie = subs(out,[sym("n")],0:200);
p=plot(Serie);
ax = gca;
ax.XScale = 'linear';
ax.YLim = [-1 1];
ax.XTick = 0:50:200; % primo elemento: incremento : ultimo elemento (primo ed ultimo entro il dominio del grafico)
xlabel('n')
ylabel('value')
```

Si ottiene questo risultato:

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_10.jpg")
</p>

Proviamo infine a rendere ora l'echo instabile, ponendo $K = 1.05$; otteniamo questo andamento divergente dell'uscita dell'echo:

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_11.jpg")
</p>

## File "allpass_echo.ino"

L'esercizio realizza l'effetto echo adottando uno schema leggermente più complesso, proposto da Manfred Robert Schroeder:

![Schroeder](https://upload.wikimedia.org/wikipedia/commons/thumb/d/df/Schr%C3%B6der%2CManfred_1993_G%C3%B6ttingen.jpg/220px-Schr%C3%B6der%2CManfred_1993_G%C3%B6ttingen.jpg)

<sup>_Manfred Robert Schroeder (1926–2009)_</sup>

e conosciuto come Schroeder all-pass filter:

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/z_18.jpg")
</p>

Per calcolarne la relazione ingresso-uscita al generico istante $n$ aggiungiamo il valore $a(n)$:

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/z_20.jpg")
</p>

Otteniamo le due relazioni seguenti:

(31) $a(n) = x(n) - KFa(n-D)$

(32) $y(n) = Fa(n-D) + Ka(n)$

Calcolando la trasformata Z di entrambi i membri delle due uguaglianze, otteniamo:

(33) $A(z) = X(z) - KFz{-D}A(z)$

(34) $Y(z) = Fz{-D}A(z) + KA(z)$

Dalla (33) ricaviamo:

(35) $A(z) = X(z) / (1 + KFz{-D})$

da cui ricaviamo, dopo alcuni passaggi:

(36) $Y(z) = X(z) (K + Fz^{-D})/(1 + KFz^{-D}) 

da cui:

(37) $H(z) = K(1 + F/Kz^{-D})/(1 + KFz^{-D}) = K(z^D + F/K)/(z^D + KF)$

### Condizioni per la stabilità

I poli della funzione di trasferimento (37) soddisfano:

(38) $z^D = -KF$

hanno quindi modulo $\sqrt[D]{|KF|}$; la condizione di stabilità è allora:

(39) $|KF|<1$

### Studio della risposta in frequenza

Descriviamo nell'ambiente Mathworks la funzione di trasferimento $H(z)$:

```
F = 0.9;
K = 0.8;
% D = 30;

% la funzione di trasferimento razionale è descritta nella forma b(z^-1)/c(z^-1)
% dove il numeratore b = b0 + b1z^-1 + b2z^-2 + b3z^-3 .....
% e il denominatore a = a0 + a1z^-1 + a2z^-2 + a3z^-3 .....

% a, b vengono descritti tramite vettori, in cui si inseriscono i soli coefficienti b_k e a_k
b1 = [1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 F/K];
a =  [1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 K*F];

b = K * b1;
```

Le successive istruzioni consentono la visualizzazione della risposta in frequenza:

```
% Frequency response
[h,w] = freqz(b,a,1000); % 1000 punti di valutazione

% Il passaggio al dominio delle frequenze avviene con la sostituzione seguente:
% z --> e^(jῶ)
%
% dove ῶ è la pulsazione normalizzata al tempo di campionamento T; detta F la frequenza di campionamento:
% ῶ = 2π/(t/T) = 2πf/F
% 
% quindi anche:
% f = (ῶF)/2π
% 
% i valori estremi sono:
% ῶ = 0 <--> f = 0
% ῶ = π <--> 2π*f/F = π <--> f = F/2 che rappresenta la frequenza di Nyquist (massima campionabile senza perdita di informazione)

plot((w*40)/(2*pi), 20*log10(abs(h))); % 40 corrisponde alla frequenza di campionamento scalata 40ksps/1000
ax = gca;
ax.XScale = 'log';
ax.YLim = [-20 20];
ax.XTick = 0:5:20; % primo elemento: incremento : ultimo elemento (primo ed ultimo entro il dominio del grafico)
xlabel('kHz');
ylabel('Magnitude (dB)');
```

Quindi, col set di valori:

$F = 0.9$, $K = 0.8$, $D = 30$

Si ottiene la seguente risposta in frequenza:

<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_15.jpg")
</p>


### Studio della risposta all'impulso

Per calcolare la risposta all'impulso unitario utilizziamo le istruzioni seguenti:

```
F = 0.9;
K = 0.8;
D = 30;

% funzione di trasferimento
syms z
H = K*(z^D + F/K)/(z^D + K*F)

% ingresso
in = 1; % impulso unitario

% risposta nel dominio dei tempi
out=iztrans(H*in);
Serie = subs(out,[sym("n")],0:300);
p=plot(Serie);
ax = gca;
ax.XScale = 'linear';
ax.YLim = [-1 1];
ax.XTick = 0:50:300; % primo elemento: incremento : ultimo elemento (primo ed ultimo entro il dominio del grafico)
xlabel('n')
ylabel('value')
```

ottenendo:
<p align="left">
<img width="500" src="/pi_pico_echo_stereo/media/z_21.jpg")
</p>