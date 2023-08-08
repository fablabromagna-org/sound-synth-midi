# Effetto echo realizzato con Raspberry Pi Pico 

### _Un esercizio per illustrare alcuni concetti dell'elaborazione audio real-time lineare e una sua modellizzazione_

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

dallo schema a blocchi risulta che:

$y(n) = Cx(n) + Ky(n-D)$

La presenza dei parametri $C$ e $K$ è prima di tutto dovuta al fatto che la lunghezza di parola è finita (16bit), ed essi aiutano a prevenire fenomeni di saturazione numerica. Il parametro $K$ ha una ulteriore importante ricaduta, perchè da esso dipende la **stabilità** dell'algoritmo: per valori $K>1$ il calcolo diverge rapidamente in presenza del più piccolo e breve segnale in ingresso. Tratteremo il tema della stabilità nelle sezioni successive introducendo ulteriori semplici strumenti di analisi.

Il blocco delay è realizzato con un array di dimensione almeno pari a $D+1$; nel codice si usano due array, uno per canale, di dimensione 14000, in grado di memorizzare 14000 sample, pari a 14000\*25us=350ms; il valore massimo per D è quindi 13999\*25us.


### Modellizzazione

L'algoritmo riceve in input una successione di valori (sample) misurati dell'ingresso e fornisce in output un'altra successione di valori:

<p align="left">
<img width="600" src="/pi_pico_echo_stereo/media/z_0.jpg")
</p>

Chiamiamo $x(n)$ la sequenza di sample in ingresso, $y(n)$ la sequenza di sample in uscita, con $n = 0, 1, 2,$... In quale relazione stanno le due sequenze? Quale la risposta in frequenza? Cosa accade modificando i parametri C e D? Per rispondere senza ricorrere a modelli matematici occorre costruire materialmente il dispositivo, scriverne e compilarne il codice, realizzare un banco di prova con generatore di funzioni e oscilloscopio per la visualizzazione dei due segnali. In alternativa si crea un _modello matematico_, e lo si studia con simulazioni automatiche. Per le sequenze a tempo discreto, come sono le successioni regolari di campioni, è stato definito da _Laplace_ un metodo modello matematico che agevola lo studio di sistemi di elaborazione lineari (cioè costituiti da moltiplicatori per costanti, sommatori e ritardi) a tempo disceto: la _trasformata Z_.

![Laplace](https://upload.wikimedia.org/wikipedia/commons/thumb/e/e3/Pierre-Simon_Laplace.jpg/260px-Pierre-Simon_Laplace.jpg)

<sup>_Ritratto di Pierre-Simon Laplace (1749-1827)_</sup>


#### Trasformiamo una serie di campioni in una funzione
Per descrivere una successione di campioni in termini matematici, torna comodo esprimerla come una funzione; definiamo una particolare funzione discreta detta _impulso unitario_ $δ(n)$, così definita:

**$δ(n)$ vale $1$ per $n=0$ ; vale $0$ per ogni altro valore di $n$**

Se vogliamo spostare il campione unitario nella posizione $k$ la funzione diventa $δ(n-k)$; infatti:
     
**$δ(n-k)$ vale $1$ per $n-k=0$ ossia per $n=k$ ; vale $0$ per ogni altro valore di $n$**

<p align="left">
<img width="400" src="/pi_pico_echo_stereo/media/z_2.jpg")
</p>

Utilizzando la funzione impulso unitario $δ(n-k)$ possiamo descrivere una qualsiasi sequenza di campioni in forma di funzione; data infatti la sequenza di campioni:

$x(0), x(1), x(2),$ ...

possiamo rappresentarla come funzione $x(n)$, costituita da una combinazione lineare di impulsi unitari via via ritardati:

$x(n) = x(0)δ(n) + x(1)δ(n-1) + x(2)δ(n-2) +$ ...

#### Enunciamo la traformata Z ed applichiamola alla serie di campioni
La trasformata Z altro non è che una semplice applicazione sulla successione $x(n)$. Definiamo $X(z)$ _trasformata Z di $x(n)$_ la funzione che si ottiene sostituendo $δ(n-k)$ con $z^{-k}$:

$X(z) = x(0)z^0 + x(1)z^{-1} + x(2)z^{-2} +$ ...

e ricordando che per qualsiasi valore $z$ si ha $z^0 = 1$:

$X(z) = x(0) + x(1)z^{-1} + x(2)z^{-2} +$ ...

Si noti che $Δ(z)$, trasformata Z di $δ(n)$, vale semplicemente 1; infatti in questo caso:

$Δ(z) = 1 + 0z^{-1} + 0z^{-2} +$ ... $= 1$

Infine, se consideriamo la generica successione $r(n)$ ottenuta ritardando la serie $y(n)$ di $D$ campioni:

$r(n) = 0δ(n) + 0δ(n-1) + 0δ(n-2) +$ ... $+ x(0)δ(n-D) + x(1)δ(n-D-1) + x(2)δ(n-D-2) +$ ... $= x(0)δ(n-D) + x(1)δ(n-D-1) + x(2)δ(n-D-2) +$ ...

Trasformando $r(n)$, otteniamo:

$R(z) = x(0)z^{n-D} + x(1)z^{n-D-1} + x(2)z^{n-D-2} +$ ... $= z^{-D}(x(0)δ(n) + x(1)δ(n-1) + x(2)δ(n-2) +$ ... $) = z^{-D}X(Z)$

Cioé: la trasformata Z di una successione $x(n)$ ritardata di $D$ campioni si ottiene moltiplicando $X(z)$ per $z^{-D}$.

Infine, altra importante proprietà della trasformata Z: date due successioni, $x(n)$ ed $y(n)$ e la successione somma $s(n)=x(n)+y(n)$, la trasformata Z di $s(n)$ vale:

$S(z) = X(z) + Y(z)$


#### Calcoliamo la funzione di trasferimento Z del nostro echo
Abbiamo visto la relazione ingresso uscita dell'algoritmo per l'echo:

$y(n) = Cx(n) + Ky(n-D)$

che rappresenta un'uguaglianza tra due successioni; applicando la trasformata Z ad entrambi i membri otteniamo:

$Y(z) = CX(z) + Kz^{-D}Y(z)$

da cui:

$Y(z) = X(z) * C/(1 - Kz^{-D})$

Definiamo il secondo termine di questa moltiplicazione **funzione di trasferimento H(z)** del nostro echo:

$H(z) = C/(1 - Kz^{-D})$

Infine scriviamo:

$Y(z) = X(z)H(z)$

<p align="left">
<img width="300" src="/pi_pico_echo_stereo/media/z_3.jpg")
</p>

Attraverso questa serie di passaggi abbiamo trasformato la relazione che lega ingresso e uscita in modo **implicito**, in una nuova relazione ingresso-uscita più astratta ma **esplicita**.



#### Cosa ne facciamo di H(z)? Condizioni per la stabilità
La conoscenza della funzione di trasferimento dell'echo consente per prima cosa di studiarne la stabilità. Le funzioni di trasferimento ricavate da algoritmi lineari sono **funzioni razionale fratte** in $z$, cioè esprimibili con un numeratore $A(z)$ ed un denominatore $B(z)$ che sono polinomi in $z$:

$H(z) = A(z)/B(z)$

Siano $A(z)$ e $B(z)$ polinomi di grado rispettivamente $N$ e ed $M$:

$A(z) = a_Nz^N + a_{N-1}z^{N-1} + a_{N-2}z^{N-2} +$ ..... $+ a_0$
$B(z) = b_Mz^M + b_{M-1}z^{M-1} + b_{M-2}z^{M-2} +$ ..... $+ b_0$

Siano $r_0, r_1, r_2$ ... $r_{N-1}$ le $N$ radici del polinomio $A(z)$; possiamo scrivere:
$A(z) = a_N(z - r_0)(z - r_1)(z - r_2)$ ... $(z - r_{N-1})$

Analogamente, siano $p_0, p_1, p_2$ ..... $p_{M-1}$ le $M$ radici del polinomio $B(z)$; scriviamo:
$B(z) = b_M(z - p_0)(z - p_1)(z - p_2)$ ... $(z - p_{M-1})$

Definiamo **_poli_** della funzione $H(z)$ gli $M$ valori $p_0, p_1, p_2$ ... $p_{M-1}$; si dimostra che, dato un algoritmo/sistema descritto dalla funzione di trasferimento discreta $H(z)$, se $H(z)$ presenta almeno un polo di valore assoluto maggiore o uguale ad 1, allora il sistema ha un comportamento **instabile**, ossia la sua uscita diverge o oscilla indipendentemente dall'ingresso; diversamente, il sistema ha un comportamento **stabile**.

Scriviamo ora la funzione di trasferimento del nostro echo nella forma $A(z)/B(z)$, moltiplicamndo numeratore e denominatore per $z^D$:

$H(z) = Cz^D/(z^D - K)$

I poli di $H(z)$ sono i valori di $z$ per cui di $(z^D - K)$ si annulla, cioè per cui vale:

$z^D = K$

Si tratta di una particolare equazione di grado $D$ in $z$ (per approfondimenti: https://www.unife.it/ing/informazione/analisi-matematica-Ib/lezioni-ed-esercizi/lezione-4-radici-n-esime-in-campo-complesso), le cui $D$ radici hanno lo stesso modulo $\sqrt[D]{|K|}$, che è un numero minore di 1 solo se e solo se $|K|<1$: ciò significa che il nostro echo è stabile se e solo se $|K|<1$.


#### Cosa ne facciamo di H(z)? Studio della risposta ad un segnale di ingresso (INCOMPLETO)
Utilizzando lo strumento di calcolo automatico online Mathworks (https://matlab.mathworks.com/) disponibile gratuitamente per un uso limitato a max 20h/mese, possiamo visualizzare, ad esempio, la risposta dell'echo ad un ingresso impulsivo.


#### Cosa ne facciamo di H(z)? La risposta del sistema ad un qualsiasi segnale
E' importante notare che la risposta $Y(z)$ di un sistema con funzione di trasferimento discreta $H(z)$ ad un ingresso impulsivo $δ(n)$ la cui trasformata vale $1$ corrisponde a $H(z)$:

$Y(z) = H(z)$




