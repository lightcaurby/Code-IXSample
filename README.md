```mermaid
classDiagram

class IIXCallback
<<interface>> IIXCallback

class IIXTimestampManager
<<interface>> IIXTimestampManager
IIXTimestampManager : Commit(Timestamp)
class TimestampManager
<<service>> TimestampManager
IIXTimestampManager <|-- TimestampManager

class IIXIndexingEngine 
<<interface>> IIXIndexingEngine
IIXIndexingEngine : Index(CIXItem)
class IndexingEngine
<<service>> IndexingEngine
IIXIndexingEngine <|-- IndexingEngine

class IIXDataRetrieval
<<interface>> IIXDataRetrieval
IIXDataRetrieval : RetrieveData(Timestamp) 
class DataRetrieval
<<service>> DataRetrieval
IIXDataRetrieval <|-- DataRetrieval

class IIXMonitor
<<interface>> IIXMonitor
IIXMonitor : RecordCommon(...)
IIXMonitor : RecordPreBatch()...)
IIXMonitor : RecordPostBatch(...)
class MonitorRelay
<<service>> MonitorRelay
IIXMonitor <|-- MonitorRelay

IIXCallback <|-- CIXCallback
CIXCallback o-- IIXDataRetrieval
CIXCallback o-- IIXIndexingEngine
CIXCallback o-- IIXTimestampManager
CIXCallback o-- IIXMonitor

class Indexer
<<service>> Indexer

Indexer : Run()
Indexer --> IIXCallback : provide
Indexer o-- DataRetrieval
Indexer o-- MonitorRelay
Indexer o-- IndexingEngine
Indexer o-- TimestampManager


```

---

```mermaid
classDiagram

class IIXCallback
<<interface>> IIXCallback


class TAIXJob
<<aspect>> TAIXJob

class IIXJob
<<interface>> IIXJob
IIXJob : Run()
IIXJob <|-- CIXJob
CIXJob : Create( IXCallback )
CIXJob : Process( CIXItem)
TAIXJob <|-- CIXJob
TAIXJob : RunImpl()
TAIXJob -- CAIXJobCombined : specialized
CAIXJobCombined : RunImpl()
TAIXJob -- CAIXJobDtSearch : specialized
CAIXJobDtSearch : RunImpl()
CAIXJobDtSearch : getNextDoc()
CAIXJobDtSearch : rewind()
CAIXJobDtSearch : OnProgressUpdate()
CAIXJobBase <|-- TAIXJob
CAIXJobBase : Reset( ...)
class CAIXJob
<<interface>> CAIXJob
CAIXJob : Reset(...)
CAIXJob : RunImpl()
CAIXJob : Process(CIXItem)
CAIXJob <|-- CAIXJobBase


class IIXEnumerable
<<interface>> IIXEnumerable
IIXEnumerable : MoveNext( Timestamp ) CIXAvailability
IIXEnumerable : Current() CXItem
IIXEnumerable : Reset()
IIXEnumerable <|-- CIXItemsEnumerator
CAIXJobBase o-- IIXEnumerable

class DIndexJob
<<dtSearch>> DIndexJob
DIndexJob <|-- CAIXJobDtSearch
DIndexJob : OnProgressUpdate
class dtsDataSource
<<dtSearch>> dtsDataSource
dtsDataSource : getNextDoc()
dtsDataSource : rewind()
dtsDataSource <|-- CAIXJobDtSearch

class Indexer
<<service>> Indexer

Indexer : Run()
Indexer --> IIXCallback : provide
Indexer o-- IIXJob : for each request


```

---

```mermaid
classDiagram

class IIXEnumerable
<<interface>> IIXEnumerable
IIXEnumerable : MoveNext( Timestamp ) CIXAvailability
IIXEnumerable : Current() CXItem
IIXEnumerable : Reset()
IIXEnumerable <|-- CIXItemsEnumerator
IIXEnumerable <|-- CIXItemsBatched
IIXEnumerable <|-- CIXItemsChunked

class IIXTimestampManager
<<interface>> IIXTimestampManager
IIXTimestampManager : Commit(Timestamp)

class IIXDataRetrieval
<<interface>> IIXDataRetrieval
IIXDataRetrieval : RetrieveData(Timestamp) 
class DataRetrieval
<<service>> DataRetrieval
IIXDataRetrieval <|-- DataRetrieval

CIXItemsEnumerator o-- CIXItemsBatched
CIXItemsEnumerator : Create( IIXCallback )

CIXItemsBatched o-- CIXItemsChunked
CIXItemsBatched : Create( IIXCallback )
CIXItemsBatched --> IIXTimestampManager : Commit

CIXItemsChunked o-- "*" CIXItem
CIXItemsChunked : Create( IIXCallback )
CIXItemsChunked --> IIXDataRetrieval : Retrieve data



```