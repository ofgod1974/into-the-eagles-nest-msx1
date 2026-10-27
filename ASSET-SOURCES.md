# Provenienza e conversione delle risorse

Le prime due versioni del prototipo usavano grafica schematica disegnata appositamente. Questa versione la sostituisce con risorse estratte dai materiali sotto indicati.

## Fondali e pianta del castello C64

- Fonte: [Into The Eagle's Nest — C64 Map, VGMaps](https://www.vgmaps.com/Atlas/C64/IntoTheEagle%27sNest-Map.png).
- Mappa composta/riprodotta da **FreluJean**, 2006-01 v1.0, attribuzione presente nell'immagine.
- Gioco e grafica originali: Pandora / Interceptor / Mindscape, 1987; Robin Chapman e autori originali.
- Copia locale: `assets/source/c64-map-frelujean.png`.
- Origini dei quattro piani nell'atlante: X=10, Y=311 / 1454 / 2597 / 3740; ogni piano 1024x1024 pixel nell'immagine ingrandita.
- Si rimuove l'ingrandimento 2x e si separano 64x64 caratteri da 8x8: emergono 152 tile distinti. La pianta e gli arredi sono quelli dell'atlante, non stanze inventate.
- I quattro frammenti del giocatore fotografato nella stanza iniziale sono rimossi dal fondale per evitare un secondo personaggio statico.
- Le annotazioni/oggetti presenti nell'atlante non costituiscono una ricostruzione verificata della logica originale di ogni oggetto.

## Personaggi animati Amiga

- Fonte: [hitchhikr/eaglenest](https://github.com/hitchhikr/eaglenest), disassemblazione completa, corretta e migliorata da **Franck "hitchhikr" Charlet**, 2023.
- Copie locali: `assets/source/gfx`, `blocks`, `maps`, `maps2`, `eagle-amiga.asm`.
- Il file gfx contiene 256 caratteri 16x8 a quattro piani, righe interlacciate. Le tabelle **YCHRS** e **NMECHR** compongono i fotogrammi del protagonista e delle guardie con sei caratteri per fotogramma.
- I fotogrammi originali 32x24 sono adattati orizzontalmente al formato 16x24 degli sprite MSX1; il file gfx originale resta conservato senza riduzioni. Sono decodificati 12 fotogrammi del protagonista e 8 delle guardie; il gioco usa gli 8 fotogrammi di camminata per ciascuno (due per direzione).
- I colori 0, 7 e 9 incorporano il pavimento nei caratteri Amiga e vengono esclusi dalle maschere dei personaggi. Le sagome sono divise in quattro maschere (uniforme, pelle/arma, luci e dettagli scuri) e in due parti verticali per gli sprite MSX1 16x16.
- La licenza della disassemblazione e conservata in `assets/source/AMIGA-DISASSEMBLY-LICENSE.txt`. Le attribuzioni del gioco originale restano degli aventi diritto; la disponibilita dei file non viene presentata come una nuova licenza sugli asset commerciali.

## Vincoli MSX1

SCREEN 2 usa due colori per ogni riga di otto pixel. La conversione seleziona una coppia di colori per riga; usa una trama per approssimare il grigio scuro assente nella palette MSX1. Non aumenta artificialmente la risoluzione e non introduce funzioni MSX2.

Il compositore degli sprite rispetta un massimo di quattro sprite per scanline: mantiene prima le sagome e i proiettili, poi aggiunge dettagli e luci dove possibile. Alcuni dettagli possono quindi mancare quando piu personaggi sono allineati.

## Riproducibilita

La colonna sonora C64 usa la trascrizione ABC di **Stephan Pabst (Stephan64)**, pubblicata il 31 luglio 2021 su [C64-Wiki](https://www.c64-wiki.de/wiki/Into_the_Eagle%27s_Nest#Theme): [file ABC](https://www.c64-wiki.de/images/0/0c/IntoEaglesNest_Theme.abc.txt). La pagina attribuisce la musica del gioco C64 a **Keith Harvey**. La trascrizione e indicata dalla fonte come ricostruita registrando gli accessi al SID; viene conservata integralmente in `assets/source/IntoEaglesNest_Theme.abc.txt`. Il port usa il tema melodico documentato in quella trascrizione, non il secondo tema alternativo non trascritto. `scripts/import-original-music.cjs` mantiene le tre voci, le note e il ritmo; SCC/PSG sostituiscono i timbri SID.

1. `node scripts/extract-source-assets.cjs` ricava tiles, piante e fotogrammi dai file originali archiviati.
2. `node scripts/import-original-assets.cjs` converte la grafica, comprime i quattro piani, verifica il round-trip delle mappe e scrive `src/original_assets.h` e le anteprime.
3. `./scripts/build.ps1` compila il gioco e crea la ROM. Gli header generati sono inclusi: Node non e necessario per una normale compilazione.

Le PNG in `assets/previews` rappresentano i dati convertiti effettivamente presenti nella ROM. Le schermate `preview*.png` alla radice del progetto provengono invece dall'emulatore.
