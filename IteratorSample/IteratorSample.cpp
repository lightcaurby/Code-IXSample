// IteratorSample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <map>
#include <iostream>
#include <vector>
#include <exception>
#include <typeinfo>

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

    // Destructor.
    virtual ~CMF_LogicalTimestamp()
    {
    }

    // Returns the timestamp value.
    int Get() const { return m_iValue;  }

    // Semantic comparison.
    bool IsLaterThan( const CMF_LogicalTimestamp& ltOther ) const
    {
        return Get() > ltOther.Get();
    }

    // Updates if the provided is later.
    void UpdateIfLater( const CMF_LogicalTimestamp& ltOther )
    {
        if( ltOther.IsLaterThan( *this ) )
            *this = ltOther;
    }

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
        return std::move( up );
    }
};
#define IX_TRY( res ) IX_TRY_IMPL( res, __LINE__ )
#define IX_UP_TRY( up ) IX_UP_TRY_IMPL( std::move( up ), __LINE__ )

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

    // Destructor.
    virtual ~IIXIndexing()
    {
    }
 };

// Helper class for reporting object lifecycles.
class CLifeReporter
{
public:

    // Constructor.
    CLifeReporter()
    {
    }

    // Destructor.
    virtual ~CLifeReporter()
    {
    }

    // Tracks a constructor call.
    static void ConstructorCalled( const char* pszClass )
    {
        // Track.
        s_mapCreated[ pszClass ]++;
    }

    // Tracks a destructor call.
    static void DestructorCalled( const char* pszClass )
    {
        // Track.
        s_mapDestroyed[ pszClass ]++;
    }

    // Runs the report.
    static void Report()
    {
        // Find out the longest class name.
        size_t stMaxLen = 0;
        for( const auto& p : s_mapCreated )
            stMaxLen = std::max( stMaxLen, p.first.length() );
        size_t stMaxLogicalIndents = ( stMaxLen + 1 ) / 3;

        // Loop through creations.
        cout << endl;
        for( const auto& p : s_mapCreated )
        {
            // Locals.
            string szClass = p.first;
            int iConstructions = p.second;
            int iDestructions = 0;
            int iSaldo = iConstructions;

            // Try to find matching destructions.
            map< string, int >::const_iterator q = s_mapDestroyed.find( szClass );
            if( q != s_mapDestroyed.end() )
            {
                // Collect the information.
                iDestructions = q->second;
                iSaldo = iConstructions - iDestructions;

            }  // end if

            // Output the info.
            size_t stAdditionalIndentsRequired = stMaxLogicalIndents - ( ( szClass.length() + 1 ) / 3 );
            cout << szClass << ":" <<
                    Indent( stAdditionalIndentsRequired ) <<
                    "\t" << iSaldo << " remaining" << 
                    "\t" << iConstructions << " up, " << 
                    "\t" << iDestructions << " down." <<
                    endl;

        }  // end for
    }

protected:
    static map< string, int > s_mapCreated;  // Creations by class.
    static map< string, int > s_mapDestroyed;  // Destructions by class.
};

// Initialization of static members.
map< string, int > CLifeReporter::s_mapCreated;
map< string, int > CLifeReporter::s_mapDestroyed;

// Helper class for tracking object lifecycles.
template< typename T >
class CLifeReporterAgent
{
public:

    // Constructor.
    CLifeReporterAgent()
    {
        // Track.
        CLifeReporter::ConstructorCalled( typeid( T ).name() );  // void
    }

    // Destructor.
    virtual ~CLifeReporterAgent()
    {
        // Track.
        CLifeReporter::DestructorCalled( typeid( T ).name() );  // void
    }
};

// Indexing engine implementation.
class CIXIndexing : public IIXIndexing, public CLifeReporterAgent< CIXIndexing >
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

// Data retrieval interface.
class IIXDataRetrieval
{
public:

    // Helper types.
    typedef shared_ptr< IIXDataRetrieval > SHP;

    // Returns the number of items globally available.
    virtual int GetGloballyAvailable() = 0;

    // Retrieves data.
    virtual CResult< CMF_LogicalTimestamp > RetrieveData(
        const CMF_LogicalTimestamp& ltLatestSeen,
        int iCount,
        OUT vector< CIXItem >& vecItems
    ) = 0;

    // Destructor.
    virtual ~IIXDataRetrieval()
    {
    }
};

// Helper class to encapsulate the data availability status during enumeration.
class CIXAvailability
{
public:

    // Availability status values.
    enum class Available { Yes, Perhaps, No };

    // Constructor.
    CIXAvailability( Available available, const CMF_LogicalTimestamp& ltLatestVisible ) :
        m_available( available ), m_ltLatestVisible( ltLatestVisible )
    {
    }

    // Default constructor.
    CIXAvailability() :
        m_available( Available::No )
    {
    }

    // Accesses the availability status.
    Available AccessAvailability() const { return m_available;  }

    // Accesses the latest visible timestamp value.
    const CMF_LogicalTimestamp& AccessLatestVisibleTimestamp() const { return m_ltLatestVisible; }

private:
    Available m_available;  // Availability status.
    CMF_LogicalTimestamp m_ltLatestVisible;  // Latest visible timestamp.
};

// Data retrieval implementation.
class CIXDataRetrieval : public IIXDataRetrieval, public CLifeReporterAgent< CIXDataRetrieval >
{
public:

    // Constructor.
    CIXDataRetrieval()
        : m_iTotalCount( 0 )
    {
    }

    // Destructor.
    virtual ~CIXDataRetrieval()
    {
    }

// IIXDataRetrieval
public:

    // Returns the number of items globally available.
    virtual int GetGloballyAvailable() override
    {
        return 81;
    }

    // Retrieves data.
    virtual CResult< CMF_LogicalTimestamp > RetrieveData(
        const CMF_LogicalTimestamp& ltLatestSeen,
        int iCount,
        OUT vector< CIXItem >& vecItems
    ) override
    {
        // Reset out params.
        vecItems.clear();

        // Latest visible timestamp after the initial threshold.
        CMF_LogicalTimestamp ltLatestVisible = ltLatestSeen;

        // Determine the timestamp threshold.
        int iStart = ltLatestSeen.Get() + 1;

        // Retrieve the data.
        vecItems.reserve( iCount );
        for( int iItem = iStart; iItem < iStart + iCount; iItem++ )
        {
            // Check the overall availability of data.
            if( m_iTotalCount >= GetGloballyAvailable() )
                break;

            // Store the data.
            ltLatestVisible = CMF_LogicalTimestamp( iItem );
            vecItems.push_back( CIXItem( iItem * 2, iItem * 3, iItem * 4, ltLatestVisible ) );

            // Keep track of the count.
            m_iTotalCount++;

        }  // end for

        // Debug output.
        cout << Indent( 2 ) <<
                "Retrieved " <<
                vecItems.size() <<
                " items. Latest visible timestamp is " <<
                ltLatestVisible.Get() <<
                "." <<
                endl;

        return CResult< CMF_LogicalTimestamp >( true, ltLatestVisible );
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

    // Accesses the latest seen timestamp.
    virtual const CMF_LogicalTimestamp& AccessLatestSeen() const = 0;

    // Updates the locally stored timestamp if the provided one is later.
    virtual void UpdateIfLater( const CMF_LogicalTimestamp& ltProvided ) = 0;

    // Accesses the data retrieval.
    virtual const IIXDataRetrieval::SHP AccessDataRetrieval() = 0;

    // Accesses the indexing engine.
    virtual const IIXIndexing::SHP AccessIndexing() = 0;

    // Destructor.
    virtual ~IIXCallback()
    {
    }
};

// Callback implementation.
class CIXCallback : public IIXCallback, public CLifeReporterAgent< CIXCallback >
{
public:

    // Constructor.
    CIXCallback( IIXDataRetrieval::SHP shpDataRetrieval, IIXIndexing::SHP shpIndexing, const CMF_LogicalTimestamp& ltLatestSeen )
        : m_shpDataRetrieval( shpDataRetrieval ), m_shpIndexing( shpIndexing ), m_ltLatestSeen( ltLatestSeen )
    {
    }

    // Destructor.
    virtual ~CIXCallback()
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

    // Accesses the latest seen timestamp.
    virtual const CMF_LogicalTimestamp& AccessLatestSeen() const override
    {
        // Access the timestamp.
        return m_ltLatestSeen;
    }

    // Updates the locally stored timestamp if the provided one is later.
    virtual void UpdateIfLater( const CMF_LogicalTimestamp& ltProvided ) override
    {
        // Delegate to the timestamp.
        m_ltLatestSeen.UpdateIfLater( ltProvided );  // void
    }

    // Accesses the data source.
    virtual const IIXDataRetrieval::SHP AccessDataRetrieval() override
    {
        // Access the data retrieval.
        return m_shpDataRetrieval;
    }

    // Accesses the indexing engine.
    virtual const IIXIndexing::SHP AccessIndexing() override
    {
        // Access the data source.
        return m_shpIndexing;
    }

private:
    IIXDataRetrieval::SHP m_shpDataRetrieval;  // Data retrieval interface.
    IIXIndexing::SHP m_shpIndexing;  // Indexing engine interface.
    CMF_LogicalTimestamp m_ltLatestSeen;  // Latest seen timestamp.
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
    virtual CResult< CIXAvailability > MoveNext( const CMF_LogicalTimestamp& ltLatestSeen ) = 0;
    
    // Gets the current item.
    virtual CResult< CIXItem > Current() const = 0;

    // Resets the enumerator.
    virtual void Reset( IIXCallback::SHP shpCB ) = 0;

    // Destructor.
    virtual ~IIXEnumerable()
    {
    }
};

// Enumerator object for chunked item data.
class CIXItemsChunked : public IIXEnumerable, public CLifeReporterAgent< CIXItemsChunked >
{
public:

    // Factory method.
    static IIXEnumerable::UP Create( IIXCallback::SHP shpCB )
    {
        // Sanity check.
        if( shpCB == nullptr )
            return IIXEnumerable::UP();

        // Delegate.
        return IIXEnumerable::UP( static_cast< IIXEnumerable* >( new CIXItemsChunked( shpCB ) ) );
    }

    // Destructor.
    virtual ~CIXItemsChunked()
    {
    }

// IIXEnumerable
public:

    // Proceeds the enumerator.
    virtual CResult< CIXAvailability > MoveNext( const CMF_LogicalTimestamp& ltLatestSeen ) override
    {
        // Initialize or proceed the enumerator. 
        CIXAvailability retval( CIXAvailability::Available::No, ltLatestSeen );
        if( m_bRetrieved == false )
        {
            // Retrieve the data as we have not done that yet.
            retval = RetrieveData( ltLatestSeen );
        }
        else if( m_itr != m_vecItems.end() )
        {
            // We have already attempted to retrieve some data.

            // Proceed with the iterator.
            m_itr++;

            // Determine the continuation status.
            retval = m_itr != m_vecItems.end()
                    ? CIXAvailability( CIXAvailability::Available::Yes, m_ltLatestVisible )
                    : m_vecItems.size() < m_shpCB->GetChunkSize()
                            ? CIXAvailability( CIXAvailability::Available::No, m_ltLatestVisible )
                            : CIXAvailability( CIXAvailability::Available::Perhaps, m_ltLatestVisible );
        }  // end if

        // Inspect the availability status.
        switch( retval.AccessAvailability() )
        {
        case CIXAvailability::Available::Perhaps:

            // Debug output.
            cout << Indent( 2 ) << "Perhaps more data available." << endl;
            break;

        case CIXAvailability::Available::No:

            // Debug output.
            cout << Indent( 2 ) << "No more data available." << endl;
            break;

        default:
            break;

        }  // end switch

        // Return value.
        return CResult< CIXAvailability >( true, retval );
    }

    // Gets the current item.
    virtual CResult< CIXItem > Current() const override
    {
        // Do we have data available?
        if( m_itr == m_vecItems.end() )
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

    // Resets the enumerator.
    virtual void Reset( IIXCallback::SHP shpCB ) override
    {
        // Reset the members.
        _ASSERTE( shpCB );
        m_bRetrieved = false;
    }

private:

    // Delete the default constructor.
    CIXItemsChunked() = delete;

    // Constructor.
    CIXItemsChunked( IIXCallback::SHP shpCB ) :
        m_shpCB( shpCB )
    {
        // Delegate.
        Reset( m_shpCB );
    }

    // Attempts to retrieve data to the local container.
    CIXAvailability RetrieveData( const CMF_LogicalTimestamp& ltLatestSeen )
    {
        // Use the data source if available.
        _ASSERTE( m_shpCB );
        m_ltLatestVisible = ltLatestSeen;
        CIXAvailability retval( CIXAvailability::Available::No, m_ltLatestVisible );
        IIXDataRetrieval::SHP shpDataRetrieval = m_shpCB->AccessDataRetrieval();
        if(shpDataRetrieval)
        {
            // Retrieve the data and set the iterator.
            m_ltLatestVisible = IX_TRY( shpDataRetrieval->RetrieveData( ltLatestSeen, m_shpCB->GetChunkSize(), OUT m_vecItems ) );
            m_itr = m_vecItems.begin();

        }  // end if

        // Initialization status.
        m_bRetrieved = true;

        // Continuation status.
        if( m_ltLatestVisible.IsLaterThan( ltLatestSeen ) )
        {
            retval = CIXAvailability( 
                    m_itr != m_vecItems.end() ? CIXAvailability::Available::Yes : CIXAvailability::Available::Perhaps,
                    m_ltLatestVisible );
        }

        return retval;
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    bool m_bRetrieved;  // Indicates whether the data has been retrieved.
    CMF_LogicalTimestamp m_ltLatestVisible;  // Latest visible timestamp.
    vector< CIXItem > m_vecItems;  // Local container for items.
    vector< CIXItem >::const_iterator m_itr;  // Local iterator for items.
};

// Enumerator object for batched item data.
class CIXItemsBatched : public IIXEnumerable, public CLifeReporterAgent< CIXItemsBatched >
{
public:

    // Factory method.
    static IIXEnumerable::UP Create( IIXCallback::SHP shpCB )
    {
        // Sanity check.
        if( shpCB == nullptr )
            return IIXEnumerable::UP();

        // Delegate.
        return IIXEnumerable::UP( static_cast< IIXEnumerable* >( new CIXItemsBatched( shpCB ) ) );
    }

    // Destructor.
    virtual ~CIXItemsBatched()
    {
    }

// IIXEnumerable
public:

    // Proceeds the enumerator.
    virtual CResult< CIXAvailability > MoveNext( const CMF_LogicalTimestamp& ltLatestSeen ) override
    {
        // Proceed the enumerator.
        CResult< CIXAvailability > res = m_upLowerLayerEnum->MoveNext( ltLatestSeen );

        // Track the return value.
        CIXAvailability availability = IX_TRY( res );
        bool bCommittedSuccessfully = false;
        switch( availability.AccessAvailability() )
        {

        // Data available.
        case CIXAvailability::Available::Yes:

            // Track the number of the received items.
            m_iCurrentCount++;
            break;

        // No data available.
        case CIXAvailability::Available::No:

            // Data source has been exhausted, so we need to commit the status
            // if we have received any items.
            // TODO: FastForward?
            if( m_iCurrentCount > 0 )
                bCommittedSuccessfully = IX_TRY( Commit( ltLatestSeen ) );
            break;

         // Perhaps data available.
        case CIXAvailability::Available::Perhaps:

            // There might be more data available, but in order to determine that we 
            // need to re-initialize the lower layers when we proceed the enumerator
            // next time. Before doing that, we must commit the current status if we
            // have received more items than the batch size.
            if( m_iCurrentCount > 0 && m_iCurrentCount >= m_shpCB->GetBatchSize() )
            {
                // Commit the current status.
                bCommittedSuccessfully = IX_TRY( Commit( ltLatestSeen ) );
            }
            else
            {
                // Forward the timestamp.
                m_shpCB->UpdateIfLater( availability.AccessLatestVisibleTimestamp() );  // void

                // Reset the lower layers.
                Reset( m_shpCB );  // void

                // Recurse.
                res = MoveNext( m_shpCB->AccessLatestSeen() );

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

    // Resets the enumerator.
    virtual void Reset( IIXCallback::SHP shpCB ) override
    {
        // Create the lower enumerator layer.
        cout << Indent( 1 ) << "Chunk being initialized." << endl;
        m_upLowerLayerEnum = IX_UP_TRY( CIXItemsChunked::Create( shpCB ) );
    }

private:

    // Delete the default constructor.
    CIXItemsBatched() = delete;

    // Constructor.
    CIXItemsBatched( IIXCallback::SHP shpCB ) :
        m_shpCB( shpCB ), m_iCurrentCount( 0 )
    {
        // Delegate.
        Reset( m_shpCB );  // void
    }

    // Commits the current progress.
    CResult< bool > Commit( const CMF_LogicalTimestamp& lt )
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
        return CResult< bool >( true, bSuccess );
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    IIXEnumerable::UP m_upLowerLayerEnum;  // The lower layer enumerator.
    int m_iCurrentCount;  // The number of the processed items.
};

// Top level enumerator object.
class CIXItemsEnumerator : public IIXEnumerable, public CLifeReporterAgent< CIXItemsEnumerator >
{
public:

    // Factory method.
    static IIXEnumerable::UP Create( IIXCallback::SHP shpCB )
    {
        // Sanity check.
        if( shpCB == nullptr )
            return IIXEnumerable::UP();

        // Delegate.
        return IIXEnumerable::UP( static_cast< IIXEnumerable* >( new CIXItemsEnumerator( shpCB ) ) );
    }

    // Destructor.
    virtual ~CIXItemsEnumerator()
    {
    }

// IIXEnumerable
public:

    // Proceeds the enumerator.
    virtual CResult< CIXAvailability > MoveNext( const CMF_LogicalTimestamp& ltLatestSeen ) override
    {

        // Delegate to the lower layer enumerator.
        CResult< CIXAvailability > res = m_upLowerLayerEnum->MoveNext( ltLatestSeen );
        CIXAvailability availability = IX_TRY( res );

        // Check the current availability first.
        if( availability.AccessAvailability() == CIXAvailability::Available::Perhaps )
        {
            // Forward the timestamp.
            m_shpCB->UpdateIfLater( availability.AccessLatestVisibleTimestamp() );  // void

            // Reset the lower layers.
            Reset( m_shpCB );  // void

            // Recurse.
            res = MoveNext( m_shpCB->AccessLatestSeen() );

        }  // end if

        return res;
    }

    // Gets the current item.
    virtual CResult< CIXItem > Current() const override
    {
        // Delegate to the lower layer.
        return m_upLowerLayerEnum->Current();
    }

    // Resets the enumerator.
    virtual void Reset( IIXCallback::SHP shpCB ) override
    {
        // Create the lower enumerator layer.
        cout << "Batch being initialized." << endl;
        m_upLowerLayerEnum = IX_UP_TRY( CIXItemsBatched::Create( shpCB ) );
    }

private:

    // Delete the default constructor.
    CIXItemsEnumerator() = delete;

    // Constructor.
    CIXItemsEnumerator( IIXCallback::SHP shpCB ) :
        m_shpCB( shpCB )
    {
        // Delegate.
        Reset( m_shpCB );  // void
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    IIXEnumerable::UP m_upLowerLayerEnum;  // The lower layer enumerator.
};

// Indexer job interface.
class IIXJob
{
public:

    // Helper types.
    typedef shared_ptr< IIXJob > SHP;

    // Helper types.
    typedef unique_ptr< IIXJob > UP;

    // Runs the job.
    virtual void Run() = 0;

    // Destructor.
    virtual ~IIXJob()
    {
    }
};

// Helper aspect interface
class IAIXJob
{
public:
    
    // Resets the aspect.
    virtual void Reset( IIXCallback::SHP shpCB ) = 0;

    // Runs the job.
    virtual void RunImpl() = 0;

    // Processes the specified item.
    virtual CResult< bool > Process( const CIXItem& item ) = 0;

    // Destructor.
    virtual ~IAIXJob()
    {
    }
};


// Base class for helper aspect classes for combined indexer jobs.
class CAIXJobBase : public IAIXJob
{
public:

    // Destructor.
    virtual ~CAIXJobBase()
    {
    }

    // Resets the aspect.
    virtual void Reset( IIXCallback::SHP shpCB ) override
    {
        // Set the member.
        m_shpCB = shpCB;

        // Create the lower enumerator layer.
        cout << "Enumerator being initialized." << endl;
        m_upLowerLayerEnum = IX_UP_TRY( CIXItemsEnumerator::Create( shpCB ) );
    }

protected:
    IIXEnumerable::UP m_upLowerLayerEnum;  // The lower layer enumerator.
    IIXCallback::SHP m_shpCB;  // Callback interface.
};

// Helper aspect class for combined indexer jobs.
class CAIXJobCombined : public CAIXJobBase
{
public:

    // Destructor.
    virtual ~CAIXJobCombined()
    {
    }

    // Runs the job.
    virtual void RunImpl() override
    {
        // Proceed with the enumerator.
        while( IX_TRY( m_upLowerLayerEnum->MoveNext( m_shpCB->AccessLatestSeen() ) ).AccessAvailability() != CIXAvailability::Available::No )
        {
            // Get the current item.
            const CIXItem& item = IX_TRY( m_upLowerLayerEnum->Current() );

            // Process the current item.
            IX_TRY( this->Process( item ) );  // Return value ignored.
        }
    }
};

// Helper aspect class for dtSearch indexer jobs.
class CAIXJobDtSearch : public CAIXJobBase
{
public:

    // Destructor.
    virtual ~CAIXJobDtSearch()
    {
    }

    // Runs the job.
    virtual void RunImpl() override
    {
    }
};

// Indexer job based on a templated aspect.
template< typename TAIXJob >
class CIXJob : public IIXJob, public TAIXJob, public CLifeReporterAgent< CIXJob< TAIXJob > >
{
public:

    // Factory method.
    static IIXJob::UP Create( IIXCallback::SHP shpCB )
    {
        // Sanity check.
        if( shpCB == nullptr )
            return IIXJob::UP();

        // Delegate.
        return IIXJob::UP( static_cast< IIXJob* >( new CIXJob( shpCB ) ) );
    }

    // Destructor.
    virtual ~CIXJob()
    {
    }

// IIXJob
public:

    // Runs the job.
    virtual void Run() override
    {
        // Delegate.
        CAIXJobCombined::RunImpl();  // void
        TAIXJob::RunImpl();  // void
    }

// AIXJob
public:

    // Processes the specified item.
    virtual CResult< bool > Process( const CIXItem& item ) override
    {
        // Try to index.
        _ASSERTE( m_shpCB );
        bool bSuccess = false;
        IIXIndexing::SHP shpIndexing = m_shpCB->AccessIndexing();
        if( shpIndexing )
            bSuccess = shpIndexing->Index( item );

        // Update the status.
        m_shpCB->UpdateIfLater( item.AccessLT() );  // void

        // Return the success status.
        return CResult< bool >( true, bSuccess );
    }

private:

    // Delete the default constructor.
    CIXJob() = delete;

    // Constructor.
    CIXJob( IIXCallback::SHP shpCB ) :
        m_shpCB( shpCB )
    {
        // Delegate.
        Reset( m_shpCB );  // void
    }

    // Resets the enumerator.
    void Reset( IIXCallback::SHP shpCB )
    {
        // Delegate to the aspect.
        TAIXJob::Reset( shpCB );
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
};

// Runs an indexing request.
void RunIndexingRequest()
{
    // Data retrieval engine.
    shared_ptr< IIXDataRetrieval > shpDataRetrieval = shared_ptr< IIXDataRetrieval >( new CIXDataRetrieval );

    // Indexing engine.
    shared_ptr< IIXIndexing > shpIndexing = shared_ptr< IIXIndexing >( new CIXIndexing );

    // Overall timestamp. Start from scratch.
    CMF_LogicalTimestamp lt;

    // Callback.
    shared_ptr< IIXCallback > shpCB = shared_ptr< IIXCallback >( new CIXCallback( shpDataRetrieval, shpIndexing, lt ) );

    // Error handling.
    try
    {
        // Initialize the job.
        cout << "Job being created." << endl;
        typedef CIXJob< CAIXJobCombined > CIXJOB;
        CIXJOB::SHP shpJob = IX_UP_TRY( CIXJOB::Create( shpCB ) );

        // Run the job.
        shpJob->Run();  // void
     }
    catch( CIXException ixex )
    {
        cout << "*** CIXException on line " << ixex.where() << endl;
        cout << ixex.what();
    }
}

// Main program.
int main()
{
    // Run an indexing request.
    RunIndexingRequest();  // void

    // Report object lifes.
    CLifeReporter::Report();  // void
}

