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