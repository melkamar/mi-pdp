# Paralelní a distribuované programování

[![Build Status](https://travis-ci.com/melkamar/mi-pdp.svg?token=vMAJz6sAMcPRgk9vRaTy&branch=master)](https://travis-ci.com/melkamar/mi-pdp)

# Poznámky

- Datový paralelismus - vyhodit task, místo toho bude parallel for, ten iterovat přes nějakou frontu/stack.
Nejdřív BFS - naplnit dostatečně frontu, pak paralelně iterovat a vyzvedávat.

## MPI
- Protokol jednodušše, na každý dotaz očekávat odpověď (žádný očekávání timeoutů atd.).

- Neposílat pointery (duh)

- Vždy zavolat `MPI_Init()` na začátku a `MPI_Finalize()` na konci.

- `MPI_Comm_size()` - kolik procesů v rámci clusteru běží (0...p-1).

- Podle ID MPI procesu rozdělení Master-Slave (Master 0, jinak slave)

- Buď počítám nebo komunikuju, nemusíme řešit nějaký asynchronní věci v rámci procesu.

- Při přijímání výsledků od slavů stačí lineárně vyřešit, netřeba řešit redukci (ale ve zkoušce říct, že to je logaritmický!)

- Při přijímání zpráv (recv) navrhnout protokol tak, abych věděl horní mez velikosti posílaných dat. Kdyby přijatá zpráva byla
větší, než buffer, tak se ořízne a zmizi! To nechceš.

- Master by měl asynchronně čekat na zprávy, když žádný nejsou, tak pracovat. Takže využít `Iprobe()`.

- Vymyslet jak rozumně posílat update nejlepšího řešení (pošle to master všem? slave co to našel všem?)

### Spuštění programu
1. Instalace MPI: `sudo apt install libopenmpi-dev`
2. Úprava Makefile - místo `g++` použít `mpiCC` (flagy a parametry zůstávají stejné)
3. Spuštění programu: `mpirun -n <počet-procesů> ./program input/graph.txt`

### Měření rychlosti - poslední cvičení
- Měřit instance co jsou sekvenčně cca 10 minut (1-20 klidně)
- Použít `qrun2` pro měření. Je to podobný jako `qrun`, jen to plánuje jinde.
- Až 60 vláken - 6 procesů, každý až 10 vláken. (CPU jsou 6core)
- Měření nedělat z bashe, ale nějakým `MPI_wtime (vtime?)` přímo z C kódu.

### Spuštění programu na STAR
1. `cp /home/mpi/{serial,parallel}_job.sh` ./
2. Upravit `serial_job.sh` a `parallel_job.sh` podle potřeby.
3. Dát do fronty. Pro detaily stačí pustit `qrun2` bez parametrů. Víc
   detailů potom tady: [EDUX](http://users.fit.cvut.cz/~soch/mi-par/run.html).
    ```bash
    # Pusť job na nodech s 20-core CPU, pusť to na 6 procesorech ve frontě pdp_long.
    qrun2 20c 6 pdp_long parallel_job.sh

    # Pusť program na jednom jádře, jedno vlákno.
    qrun2 12c 1 pdp_serial serial_job.sh
    ```

    Aplikace může získat maximálně 10 vláken na procesor, takže maximálně
    jde pustit 10×6=60 vláken.
