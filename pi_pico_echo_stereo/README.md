# Effetto echo realizzato con Raspberry Pi Pico 

### Un esercizio per illustrare alcuni concetti dell'elaborazione audio real-time

Questo semplice dispositivo realizzato con un Raspberry Pi Pico ed una coppia di DAC I2S MAX98357A, è un effetto echo stereo (anche detto delay) con campionamento 12bit - 40Ksps (samples per second); non include alcuna regolazione esterna né filtri anti aliasing agli ingressi, perchè lo scopo è puramente didattico.

Al Pi Pico è demandato il ruolo di effettuare la lettura/campionamento dei segnali in ingresso (left - right) utilizzando il convertitore ADC 12bit interno alla board, quindi eseguire le elaborazioni necessarie (scrivere nelle code FIFO i sample letti, leggere sample dalle code FIFO, effettuare moltiplicazioni e somme), infine inviare un flusso audio digitale 16bit - 40Ksps a ciascuno dei due DAC, utilizzando connessioni fisiche a 3 fili con protocollo I2S.

A loro volta i DAC effettuano una conversione da digitale a PWM, con frequenza 300Khz, utilizzando in uscita un circuito a bassa impedenza d'uscita ed elevata corrente, in grado quindi di pilotare direttamente gli altoparlanti 4-8Ω; le caratteristiche di impedenza e risposta elettromeccanica degli altoparlanti abbattono e rendono inudibili le componenti audio del flusso PWM superiori 15-20kHz.

Nel video seguente una breve registrazione dell'effetto; all'ingresso è collegato un generatore di funzioni con sweep regolato manualmente:

https://github.com/fablabromagna-org/sound-synth-midi/assets/41198542/2fde0d05-7ad2-4d5e-9d04-8c904a834ee6


#### Schema a blocchi

Lo schema ingresso-uscita seguente illustra l'articolazione dell'effetto echo nei suoi componenti: il delay ed i blocchi sommatore e moltiplicatore , l'interconnessione tra gli stessi e la direzione del flusso di elaborazione.

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/delay_0.jpg")
</p>

Lo schema nasce da una semplice intuizione: per esperienza comune un suono con eco è dato dalla somma (in aria) del suono origine, dello stesso suono riflesso (che giunge con un certo ritardo all'origine), e delle ulteriori riflessioni delle riflessioni.

Nel passaggio dallo schema a blocchi al codice occorre tener presente che il segnale trattato non è continuo ma campionato; non a tempo continuo ma a tempo discreto. Questo significa che occorre elaborare singoli campioni e fornire in uscita, analogamente, singoli campioni.

Nell'esercizio, il codice ha la totale responsabilità del rispetto del timing. La funzione "repeating_timer_callback", che viene chiamata via interrupt ogni 25us (tempo di campionamento), svolge la funzione di leggere (campionare) i due segnali in ingresso (utilizzando i convertitori AD interni alla scheda Raspberry Pi Pico), effettuare le elaborazioni audio richieste, infine inviare due campioni (sample) ai due convertitori DA esterni. 25us rappresenta il tempo concesso per effettuare l'elaborazione audio desiderata, e corrisponde ad una frequenza di campionamento pari a 40ksps.

In termini di codice, lo schema a blocchi si traduce in una singola funzione lineare; infatti se:
- D è il valore del ritardo (come numero di sample) introdotto dal blocco delay
- x(n) il sample in ingresso all'istante n
- y(n) il sample in uscita all'istante n

dallo schema a blocchi risulta che:

$y(n) = Cx(n) + Ky(n-D)$

La presenza dei parametri C e K è prima di tutto dovuta al fatto che la lunghezza di parola è finita (16bit), ed essi aiutano a prevenire fenomeni di saturazione numerica. Il parametro K ha una ulteriore importante ricaduta, perchè da esso dipende la "stabilità" dell'algoritmo: per valori K > 1 il calcolo diverge rapidamente in presenza del più piccolo e breve segnale in ingresso. Tratteremo il tema della stabilità nelle sezioni successive introducendo ulteriori semplici strumenti di analisi.

Il blocco delay è realizzato con un array di dimensione almeno pari a D+1; nel codice si usano due array, uno per canale, di dimensione 14000, in grado di memorizzare 14000 sample, pari a 14000*25us=350ms; il valore massimo per D è quindi 13999*25us.


#### Modellizzazione

L'algoritmo riceve in input una successione di valori (sample) misurati dell'ingresso e fornisce in output un'altra successione di valori:

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/z_0.jpg")
</p>

Chiamiamo x(n) la sequenza di sample in ingresso, y(n) la sequenza di sample in uscita, con n = 0, 1, 2, etc. In quale relazione stanno le due sequenze? Per verificarlo senza ricorre a modelli matematici occorre costruire materialmente il dispositivo, scriverne e compilarne il codice, definire un banco di prova con generatore di funzioni e oscilloscopio per la visualizzazione dei due segnali. In alternativa si crea un _modello matematico_, e lo si studia con simulazioni automatiche. Per le sequenze a tempo discreto, come sono le successioni regolari di campioni, è stata definito un modello matematico (da Pierre-Simon Laplace, 1749-1827) che consente l'esame di sistemi di elaborazione lineari costituiti da moltiplicatori, sommatori e ritardi: la _trafsormata Z_.


##### Trasformiamo una serie di campioni in una funzione
Per descrive una successioni di campioni in termini matematici, torna comodo esprimerla come una funzione; definiamo una particolare funzione discreta detta _impulso unitario" δ(n)_, così definita:

https://latex.codecogs.com/svg.image?\delta(n)=\left\{\begin{matrix}&plus;1&\text{se&space;n}=0\\0&\text{se&space;n}\neq&space;0\end{matrix}\right.


Vediamo ora che δ(n-D) possiamo spostare il valore 1 nella posizione D:
     
_δ(n-D) vale 1 per n-D=0 ossia per n=D, e vale 0 per ogni altro valore di n_

<p align="left">
<img width="400" src="/pi_pico_echo_stereo/media/z_2.jpg")
</p>

Utilizzando la funzione impulso unitario δ possiamo descrivere una qualsiasi sequenza di campioni x(n); data infatti la sequenza x(n)

_x(n) = x0, x1, x2, ......_

possiamo scrivere x(n) come:

_x(n) = x0δ(n) + x1δ(n-1) + x2δ(n-2) + ......_


##### Enunciamo la traformata Z ed applichiamola alla serie di campioni
Eseguiamo ora la seguente applicazione (trasformazione) sulla successione x(n), che chiamiamo _trasformata Z_ di x(n); definiamo la funzione trasformata X(z) ciò che si ottiene sostituendo a δ(n-k) il valore z^(-k); otteniamo:

_X(z) = x0z^0 + x1z^(-1) + x2z^(-2) + ....._ 

e ricordando che per qualsiasi valore z si ha z^0 = 1:

_X(z) = x0 + x1z^(-1) + x2z^(-2) + ....._

Si noti che Δ(z), trasformata Z di δ(n), vale semplicemente 1; infatti in questo caso:

_Δ(z) = 1 + 0z^(-1) + 0z^(-2) + .... = 1_

Infine, se consideriamo la generica successione r(n) ottenuta ritardando la serie y(n) di D campioni:

_r(n) = 0δ(n) + 0δ(n-1) + 0δ(n-2) + ......+ x0δ(n-D) + x1δ(n-D-1) + x2δ(n-D-2) + ...... = x0δ(n-D) + x1δ(n-D-1) + x2δ(n-D-2) + ......_

Trasformando r(n), otteniamo:

_R(z) = x0z^(n-D) + x1z^(n-D-1) + x2z^(n-D-2) + ..... = z^(-D)(x0δ(n) + x1δ(n-1) + x2δ(n-2) + ......) = z^(-D)X(Z)_

Cioé: la trasformata Z di una successione x(n) ritardata di D campioni si ottiene moltiplicando X(z) per z^(-D).

Infine, altra importante proprietà della trasformata Z: date due successioni, x(n) ed y(n) e la successione somma s(n)=x(n)+y(n), la trasformata Z di s(n) vale:

_S(z) = X(z) + Y(z)_


##### Calcoliamo la funzione di trasferimento Z del nostro echo
Abbiamo visto la relazione ingresso uscita dell'algoritmo per l'echo:

_y(n) = Cx(n) + Ky(n-D)_

che rappresenta un'uguaglianza tra due successioni; applicando la trasformata Z ad entrambi i membri otteniamo:

_Y(z) = CX(z) + Kz^(-D)*Y(z)_

da cui:

_Y(z) = X(z) * C/(1 - Kz^(-D))_

Definiamo il secondo termine di questa moltiplicazione _funzione di trasferimento H(z)_ del nostro echo:

_H(z) = C/(1 - Kz^(-D))_

da cui:

_Y(z) = X(z) * H(z)_

<p align="left">
<img width="300" src="/pi_pico_echo_stereo/media/z_3.jpg")
</p>

Attraverso questa serie di passaggi abbiamo trasformato la relazione che lega ingresso e uscita in modo _implicito_, in una nuova relazione ingresso-uscita più astratta ma _esplicita_.


##### Cosa ne facciamo di H(z)? Condizioni per la stabilità
La conoscenza della funzione di trasferimento dell'echo consente per prima cosa di studiarne la stabilità. Le funzioni di trasferimento ricavate da algoritmi lineari sono _funzioni polinomiali razionali_ in z, cioè esprimibili con un numeratore A ed un denominatore B che sono polinomi in z:

_H(z) = A(z)/B(z)_

Siano A(z) e B(z) polinomi in z, di grado rispettivamente N e ed M:

_A(z) = az^N + a1z^(N-1) + a2z^(N-2) + ..... + a[N-1]_
_B(z) = bz^M + b1z^(M-1) + b2z^(M-2) + ..... + b[M-1]_

Siano k0, k1, k2 ....k(N-1) le radici del polinomio A(z) e p0, p1, p2,....p(M-1) le radici del polinomio B(z):

_A(z) = a(z - k0)(z - k1)(z - k2)......(z - k[N-1])_
_B(z) = b(z - p0)(z - p1)(z - p2)......(z - p[M-1])_

Definiamo _poli_ della funzione H(z) i valori p0, p1, p2...pM per cui il denominatore B(z); si dimostra che, dato un algoritmo/sistema descritto dalla funzione di trasferimento H(z), se H(z) presenta almeno un polo di valore assoluto maggiore o uguale ad 1, allora il sistema ha un comportamento _instabile_, ossia la sua uscita diverge o oscilla indipendentemente dall'ingresso; diversamente il sistema di dice _stabile_.
Nel caso del nostro echo, scriviamolo nella forma A(z)/B(z), moltiplicamndo numeratore e denominatore per z^(D):

_H(z) = Cz^D/(z^D - K)_

I poli di H(z) sono i valori per cui di (z^D - K) si annulla:

_z^D = K_

si tratta di una particolare equazione di grado D in z (per approfondimenti: https://www.unife.it/ing/informazione/analisi-matematica-Ib/lezioni-ed-esercizi/lezione-4-radici-n-esime-in-campo-complesso), le cui D radici hanno lo stesso modulo |K|^(-D), che è un numero minore di 1 solo se e solo se |K|<1: ciò significa che il nostro echo è stabile se e solo se |K|<1.


##### Cosa ne facciamo di H(z)? Studio della risposta ad un segnale di ingresso (INCOMPLETO)
Utilizzando lo strumento di calcolo automatico online Mathworks (https://matlab.mathworks.com/) disponibile gratuitamente per un uso limitato a max 20h/mese, possiamo visualizzare, ad esempio, la risposta dell'echo ad un ingresso impulsivo.




##### Cosa ne facciamo di H(z)? Studio della risposta ad un segnale di ingresso (INCOMPLETO)
Sempre con lausilio di Mathworks possiamo visualizzare la risposta in frequenza dell'echo.
