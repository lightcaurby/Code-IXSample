// IteratorSample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <exception>

using namespace std;

#define IN
#define OUT

namespace
{
    // Generates indentation (each level is 3 spaces).
    string Indent( size_t stLevel )
    {
        // Generate whitespace for indentation.
        return string( stLevel * 3, ' ' );
    }
}

// Custom exception.
class CIXException : public exception
{
public:
    
    // Constructor.
    CIXException( int iLine, const char* szMessage )
        : m_iLine( iLine ), m_szMessage( szMessage )
    {
    }

    // Returns an explanatory string.
    virtual const char* what() const override
    {
        // Return from the member.
        return m_szMessage.c_str();
    }

    int where() const
    {
        return m_iLine;
    }

private:
    int m_iLine;  // Line number.
    string m_szMessage;  // Exception description.
};

// Logical timestamp.
class CMF_LogicalTimestamp
{
public:

    // Copy constructor.
    CMF_LogicalTimestamp( const CMF_LogicalTimestamp& lt )
        : m_iValue( lt.Get() )
    {
    }

    // Constructor.
    CMF_LogicalTimestamp( int iValue )
        : m_iValue( iValue )
    {
    }

    // Default constructor.
    CMF_LogicalTimestamp()
        : m_iValue( 0 )
    {
    }

    // Returns the timestamp value.
    int Get() const { return m_iValue;  }

private:
    int m_iValue;  // Timestamp value.
};

// Indexable item.
class CIXItem
{
public:

    // Constructor.
    CIXItem( int i, int j, int k, const CMF_LogicalTimestamp& lt )
        : m_i( i ),m_j( j ), m_k( k ), m_lt( lt )
    {
    }

    // Default constructor.
    CIXItem() : CIXItem( 0, 0, 0, CMF_LogicalTimestamp( 0 ) ) {}

    // Accesses the timestamp.
    const CMF_LogicalTimestamp& AccessLT() const { return m_lt;  }

    // Gets the indexable data.
    int GetI() const { return m_i;  }
    int GetJ() const { return m_j; }
    int GetK() const { return m_k; }

private:
    int m_i;  // Indexable data.
    int m_j;  // Indexable data.
    int m_k;  // Indexable data.
    CMF_LogicalTimestamp m_lt;  // Timestamp.
};

// Generic result with a built-in success code.
template< typename T >
class CResult
{
public:

    // Constructor.
    CResult( bool bSuccess, const T& tRetVal )
        : m_bSuccess( bSuccess ), m_tRetVal( tRetVal )
    {
    }

    // Default constructor.
    CResult()
        : CResult( false, T() )
    {
    }

    // Returns the success state.
    bool Success() const { return m_bSuccess;  }

    // Accesses the actual return value.
    const T& AccessRetVal() const { return m_tRetVal; }

private:
    bool m_bSuccess;  // Success.
    T m_tRetVal;  // Return value.
};

// Helpers for error handling.
namespace
{
    template< typename T >
    const T& IX_TRY_IMPL( const CResult< T >& res, int iLine )
    {
        if( res.Success() == false )
            throw CIXException( iLine, "Error" );
        return res.AccessRetVal();
    }

    template< typename T >
    unique_ptr< T > IX_UP_TRY_IMPL( unique_ptr< T > up, int iLine )
    {
        if( up == false )
            throw CIXException( iLine, "Error" );
        return up;
    }
};
#define IX_TRY( res ) IX_TRY_IMPL( res, __LINE__ )
#define IX_UP_TRY( up ) IX_UP_TRY_IMPL( up, __LINE__ )

// Indexing engine interface.
class IIXIndexing
{
public:

    // Helper types.
    typedef shared_ptr< IIXIndexing > SHP;

    // Indexes data.
    virtual bool Index( const CIXItem& item ) = 0;

    // Commits the current state.
    virtual bool Commit( const CMF_LogicalTimestamp& lt, int iActualCount ) = 0;
};

// Indexing engine implementation.
class CIXIndexing : public IIXIndexing
{
public:

    // Constructor.
    CIXIndexing()
        : m_iItemsIndexed( 0 )
    {
    }

    // Destructor.
    virtual ~CIXIndexing()
    {
        cout << endl << "Indexed " << m_iItemsIndexed << " items." << endl;
    }

    // IIXIndexing
public:

    // Indexes data.
    virtual bool Index( const CIXItem& item ) override
    {
        // Debug output.
        cout << Indent( 3 )
            << "ts( "
            << item.AccessLT().Get()
            << " ), data( "
            << item.GetI()
            << ","
            << item.GetJ()
            << ","
            << item.GetK()
            << " )"
            << " ...indexed."
            << endl;

        // Increase the overall counter.
        m_iItemsIndexed++;

        return true;
    }

    // Commits the current state.
    virtual bool Commit( const CMF_LogicalTimestamp& lt, int iActualCount ) override
    {
        // Report the commit.
        cout << "Committed at ts( " << lt.Get() << " ) after " << iActualCount << " items." << endl;
        return true;
    }

private:
    int m_iItemsIndexed;  // Number of indexed items.
};

// Data source interface.
class IIXDataSource
{
public:

    // Helper types.
    typedef shared_ptr< IIXDataSource > SHP;

    // Returns the number of items globally available.
    virtual int GetGloballyAvailable() = 0;

    // Retrieves data.
    virtual bool RetrieveData(
        const CMF_LogicalTimestamp& ltLastSeen,
        int iCount,
        OUT vector< CIXItem >& vecItems
    ) = 0;
};

// Data source implementation.
class CIXDataSource : public IIXDataSource
{
public:

    // Constructor.
    CIXDataSource()
        : m_iTotalCount( 0 )
    {
    }

// IIXDataSource
public:

    // Returns the number of items globally available.
    virtual int GetGloballyAvailable() override
    {
        return 81;
    }

    // Retrieves data.
    virtual bool RetrieveData(
        const CMF_LogicalTimestamp& ltLastSeen,
        int iCount,
        OUT vector< CIXItem >& vecItems
    ) override
    {
        // Reset out params.
        vecItems.clear();

        // Determine the timestamp threshold.
        int iStart = ltLastSeen.Get() + 1;

        // Retrieve the data.
        vecItems.reserve( iCount );
        for( int iItem = iStart; iItem < iStart + iCount; iItem++ )
        {
            // Check the overall availability of data.
            if( m_iTotalCount >= GetGloballyAvailable() )
                break;

            // Store the data.
            vecItems.push_back( CIXItem( iItem * 2, iItem * 3, iItem * 4, CMF_LogicalTimestamp( iItem ) ) );

            // Keep track of the count.
            m_iTotalCount++;

        }  // end for

        // Debug output.
        cout << Indent( 2 ) << "Retrieved " << vecItems.size() << " items." << endl;

        return true;
    }

private:
    int m_iTotalCount;  // Total number of retrieved items.
};

// Callback interface.
class IIXCallback
{
public:

    // Helper types.
    typedef shared_ptr< IIXCallback > SHP;

    // Returns the batch size.
    virtual int GetBatchSize() = 0;

    // Returns the chunk size.
    virtual int GetChunkSize() = 0;

    // Accesses the data source.
    virtual const IIXDataSource::SHP& AccessDataSource() = 0;

    // Accesses the indexing engine.
    virtual const IIXIndexing::SHP& AccessIndexing() = 0;
};

// Callback implementation.
class CIXCallback : public IIXCallback
{
public:

    // Constructor.
    CIXCallback( IIXDataSource::SHP& shpDataSource, IIXIndexing::SHP& shpIndexing )
        : m_shpDataSource( shpDataSource ), m_shpIndexing( shpIndexing )
    {
    }

// IIXCallback
public:

    // Returns the batch size.
    virtual int GetBatchSize() override
    {
        // Batch size.
        return 18;
    }

    // Returns the chunk size.
    virtual int GetChunkSize() override
    { 
        // Chunk size.
        return 10; 
    }

    // Accesses the data source.
    virtual IIXDataSource::SHP& AccessDataSource() override
    {
        // Access the data source.
        return m_shpDataSource;
    }

    // Accesses the indexing engine.
    virtual IIXIndexing::SHP& AccessIndexing() override
    {
        // Access the data source.
        return m_shpIndexing;
    }

private:
    IIXDataSource::SHP m_shpDataSource;  // Data source interface.
    IIXIndexing::SHP m_shpIndexing;  // Indexing engine interface.
};

// Enumerator interface.
class IIXEnumerable
{
public:

    // Helper types.
    typedef shared_ptr< IIXEnumerable > SHP;

    // Helper types.
    typedef unique_ptr< IIXEnumerable > UP;

    // Proceeds the enumerator.
    enum class Available { Yes, Perhaps, No };
    virtual CResult< Available > MoveNext( const CMF_LogicalTimestamp& ltLastSeen ) = 0;
    
    // Gets the current item.
    virtual CResult< CIXItem > Current() const = 0;
};

// Processor interface.
class IIXProcessor
{
public:

    // Helper types.
    typedef shared_ptr< IIXProcessor > SHP;

    // Helper types.
    typedef unique_ptr< IIXProcessor > UP;

    // Processes the specified item.
    virtual CResult< CMF_LogicalTimestamp > Process( const CIXItem& item ) = 0;
};

// Enumerator object for chunked item data.
class CIXItemsChunked : public IIXEnumerable
{
public:
    // Helper types.
    typedef unique_ptr< CIXItemsChunked > UP;

    // Factory method.
    static CIXItemsChunked::UP Create( IIXCallback::SHP& shpCB )
    {
        // Sanity check.
        if( shpCB == nullptr )
            return UP();

        // Delegate.
        return UP( new CIXItemsChunked( shpCB, shpCB->GetChunkSize() ) );
    }

// IIXEnumerable
public:

    // Proceeds the enumerator.
    virtual CResult< IIXEnumerable::Available > MoveNext( const CMF_LogicalTimestamp& ltLastSeen ) override
    {
        // Initialize or proceed the enumerator. 
        IIXEnumerable::Available retval = IIXEnumerable::Available::No;
        if(m_bRetrieved == false)
        {
            // Retrieve the data as we have not done that yet.
            retval = RetrieveData( ltLastSeen );
        }
        else if(m_itr != m_vecItems.end())
        {
            // We have already attempted to retrieve some data.

            // Proceed with the iterator.
            m_itr++;

            // Determine the continuation status.
            retval = m_itr != m_vecItems.end()
                ? IIXEnumerable::Available::Yes
                : m_vecItems.size() < m_iRetrievalCount
                ? IIXEnumerable::Available::No
                : IIXEnumerable::Available::Perhaps;
        }  // end if

        // Inspect the availability status.
        switch(retval)
        {
        case IIXEnumerable::Available::Perhaps:

            // Debug output.
            cout << Indent( 2 ) << "Perhaps more data available." << endl;
            break;

        case IIXEnumerable::Available::No:

            // Debug output.
            cout << Indent( 2 ) << "No more data available." << endl;
            break;

        default:
            break;

        }  // end switch

        // Return value.
        return CResult< IIXEnumerable::Available >( true, retval );
    }

    // Gets the current item.
    virtual CResult< CIXItem > Current() const override
    {
        // Do we have data available?
        if(m_itr == m_vecItems.end())
        {
            // No, raise an error.
            return CResult< CIXItem >( false, CIXItem() );
        }
        else
        {
            // Yes, use the iterator to return the data.
            return CResult< CIXItem >( true, *m_itr );

        }  // end if
    }

private:

    // Delete the default constructor.
    CIXItemsChunked() = delete;

    // Constructor.
    CIXItemsChunked( const IIXCallback::SHP& shpCB, int iCount ) :
        m_shpCB( shpCB ),
        m_bRetrieved( false ),
        m_iRetrievalCount( iCount )
    {
    }

    // Attempts to retrieve data to the local container.
    IIXEnumerable::Available RetrieveData( const CMF_LogicalTimestamp& ltLastSeen )
    {
        // Use the data source if available.
        _ASSERTE( m_shpCB );
        IIXEnumerable::Available retval = IIXEnumerable::Available::No;
        IIXDataSource::SHP shpDataSource = m_shpCB->AccessDataSource();
        if(shpDataSource)
        {
            // Retrieve the data and set the iterator.
            if(shpDataSource->RetrieveData( ltLastSeen, m_iRetrievalCount, OUT m_vecItems ))
                m_itr = m_vecItems.begin();

        }  // end if

        // Initialization status.
        m_bRetrieved = true;

        // Continuation status.
        if(m_itr != m_vecItems.end())
            retval = IIXEnumerable::Available::Yes;

        return retval;
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    bool m_bRetrieved;  // Indicates whether the data has been retrieved.
    int m_iRetrievalCount;  // Number of the items to retrive (if available).
    vector< CIXItem > m_vecItems;  // Local container for items.
    vector< CIXItem >::const_iterator m_itr;  // Local iterator for items.
};

// Enumerator object for batched item data.
class CIXItemsBatched : public IIXEnumerable
{
public:

    // Helper types.
    typedef unique_ptr< CIXItemsBatched > UP;

    // Factory method.
    static CIXItemsBatched::UP Create( IIXCallback::SHP& shpCB )
    {
        // Sanity check.
        if( shpCB == nullptr )
            return UP();

        // Delegate.
        return UP( new CIXItemsBatched( shpCB ) );
    }

// IIXEnumerable
public:

    // Proceeds the enumerator.
    virtual CResult< IIXEnumerable::Available > MoveNext( const CMF_LogicalTimestamp& ltLastSeen ) override
    {
        // Proceed the enumerator.
        CResult< IIXEnumerable::Available > res = m_upLowerLayerEnum->MoveNext( ltLastSeen );

        // Check the return value.
        switch( IX_TRY( res ) )
        {
        // More data available.
        case IIXEnumerable::Available::Yes:

            // Track the number of the received items.
            m_iCurrentCount++;
            break;

        // No more data available.
        case IIXEnumerable::Available::No:

            // Data source has been exhausted, so we need to commit the status
            // if we have received any items.
            if( m_iCurrentCount > 0 )
                res = Commit( ltLastSeen, res );
            break;

        // Perhaps more data available.
        case IIXEnumerable::Available::Perhaps:

            // There might be more data available, but in order to determine that we 
            // need to re-initialize the lower layers when we proceed the enumerator
            // next time. Before doing that, we must commit the current status if we
            // have received more items than the batch size.
            if(m_iCurrentCount > 0 && m_iCurrentCount >= m_shpCB->GetBatchSize())
            {
                // Commit the current status.
                res = Commit( ltLastSeen, res );
            }
            else
            {
                // Reset the lower layers.
                Initialize( m_shpCB );
                
                // Recurse.
                res = MoveNext( ltLastSeen );

            }  // end if

            break;

        default:
            break;

        }  // end switch

        return res;
    }

    // Gets the current item.
    virtual CResult< CIXItem > Current() const override
    {
        // Delegate to the lower layer.
        return m_upLowerLayerEnum->Current();
    }

private:

    // Delete the default constructor.
    CIXItemsBatched() = delete;

    // Constructor.
    CIXItemsBatched( IIXCallback::SHP& shpCB )
        : m_shpCB( shpCB ), m_iCurrentCount( 0 )
    {
        // Delegate.
        Initialize( shpCB );
    }

    // Initializes the lower enumerator layer.
    void Initialize( IIXCallback::SHP& shpCB )
    {
        // Create the lower enumerator layer.
        cout << Indent( 1 ) << "Chunk being initialized." << endl;
        m_upLowerLayerEnum = IX_UP_TRY( CIXItemsChunked::Create( shpCB ) );
    }

    // Commits the current progress.
    CResult< IIXEnumerable::Available > Commit( const CMF_LogicalTimestamp& lt, CResult< IIXEnumerable::Available >& res )
    {
        // Try to commit.
        _ASSERTE( m_shpCB );
        bool bSuccess = false;
        IIXIndexing::SHP shpIndexing = m_shpCB->AccessIndexing();
        if( shpIndexing )
            bSuccess = shpIndexing->Commit( lt, m_iCurrentCount );

        // Reset the counter for batch content.
        m_iCurrentCount = 0;

        // Return value.
        return CResult< IIXEnumerable::Available >( bSuccess, IX_TRY( res ) );
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    CIXItemsChunked::UP m_upLowerLayerEnum;  // The lower layer enumerator.
    int m_iCurrentCount;  // The number of the received items.
};

// Top level enumerator object.
class CIXItems : public IIXEnumerable, public IIXProcessor
{
public:

    // Helper types.
    typedef shared_ptr< CIXItems > SHP;

    // Helper types.
    typedef unique_ptr< CIXItems > UP;

    // Factory method.
    static CIXItems::UP Create( IIXCallback::SHP& shpCB )
    {
        // Sanity check.
        if( shpCB == nullptr )
            return UP();

        // Delegate.
        return UP( new CIXItems( shpCB ) );
    }

// IIXEnumerable
public:

    // Proceeds the enumerator.
    virtual CResult< IIXEnumerable::Available > MoveNext( const CMF_LogicalTimestamp& ltLastSeen ) override
    {
        // Move forward until the lower layers give a definite Yes/No response, because
        // the Perhaps response indicates that we need to try to get more data.
        CResult< IIXEnumerable::Available > res = m_upLowerLayerEnum->MoveNext( ltLastSeen );
        if( IX_TRY( res ) == IIXEnumerable::Available::Perhaps )
        {
            // Reset the lower layers.
            Initialize( m_shpCB );

            // Recurse.
            res = MoveNext( ltLastSeen );
        }

        return res;
    }

    // Gets the current item.
    virtual CResult< CIXItem > Current() const override
    {
        // Delegate to the lower layer.
        return m_upLowerLayerEnum->Current();
    }

// IIXProcessor
public:

    // Processes the specified item.
    virtual CResult< CMF_LogicalTimestamp > Process( const CIXItem& item ) override
    {
        // Try to index.
        _ASSERTE( m_shpCB );
        bool bSuccess = false;
        IIXIndexing::SHP shpIndexing = m_shpCB->AccessIndexing();
        if( shpIndexing )
            bSuccess = shpIndexing->Index( item );

        // Return the current timestamp.
        return CResult< CMF_LogicalTimestamp >( true, item.AccessLT() );
    }

private:

    // Delete the default constructor.
    CIXItems() = delete;

    // Constructor.
    CIXItems( IIXCallback::SHP& shpCB )
        : m_shpCB( shpCB )
    {
        // Delegate.
        Initialize( shpCB );
    }

    // Initializes the lower enumerator layer.
    void Initialize( IIXCallback::SHP& shpCB )
    {
        // Create the lower enumerator layer.
        cout << "Batch being initialized." << endl;
        m_upLowerLayerEnum = IX_UP_TRY( CIXItemsBatched::Create( shpCB ) );
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    CIXItemsBatched::UP m_upLowerLayerEnum;  // The lower layer enumerator.
};

// Main program.
int main()
{
    // Data source.
    shared_ptr< IIXDataSource > shpDataSource = shared_ptr< IIXDataSource >( new CIXDataSource );

    // Indexing engine.
    shared_ptr< IIXIndexing > shpIndexing = shared_ptr< IIXIndexing >( new CIXIndexing );

    // Callback.
    shared_ptr< IIXCallback > shpCB = shared_ptr< IIXCallback >( new CIXCallback( shpDataSource, shpIndexing ) );

    // Overall timestamp.
    CMF_LogicalTimestamp lt;

    // Error handling.
    try
    {
        // Initialize the enumerator.
        CIXItems::UP upItems = IX_UP_TRY( CIXItems::Create( shpCB ) );

        // Proceed with the enumerator.
        while( IX_TRY( upItems->MoveNext( lt ) ) != IIXEnumerable::Available::No )
        {
            // Get the current item.
            const CIXItem& item = IX_TRY( upItems->Current() );

            // Process the current item.
            lt = IX_TRY( upItems->Process( item ) );
        }

     }
    catch( CIXException ixex )
    {
        cout << "*** CIXException on line " << ixex.where() << endl;
        cout << ixex.what();
    }
}

