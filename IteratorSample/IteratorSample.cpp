// IteratorSample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <map>
#include <iostream>
#include <vector>
#include <exception>
#include <typeinfo>
#include <random>

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
class CLogicalTimestamp
{
public:

    // Copy constructor.
    CLogicalTimestamp( const CLogicalTimestamp& lt )
        : m_iValue( lt.Get() )
    {
    }

    // Constructor.
    CLogicalTimestamp( int iValue )
        : m_iValue( iValue )
    {
    }

    // Default constructor.
    CLogicalTimestamp()
        : m_iValue( 0 )
    {
    }

    // Destructor.
    virtual ~CLogicalTimestamp()
    {
    }

    // Returns the timestamp value.
    int Get() const { return m_iValue;  }

    // Semantic comparison.
    bool IsLaterThan( const CLogicalTimestamp& ltOther ) const
    {
        return Get() > ltOther.Get();
    }

    // Updates if the provided is later.
    void UpdateIfLater( const CLogicalTimestamp& ltOther )
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
    CIXItem( int i, int j, int k, const CLogicalTimestamp& lt )
        : m_i( i ),m_j( j ), m_k( k ), m_lt( lt )
    {
    }

    // Default constructor.
    CIXItem() : CIXItem( 0, 0, 0, CLogicalTimestamp( 0 ) ) {}

    // Accesses the timestamp.
    const CLogicalTimestamp& AccessLT() const { return m_lt;  }

    // Gets the indexable data.
    int GetI() const { return m_i;  }
    int GetJ() const { return m_j; }
    int GetK() const { return m_k; }

private:
    int m_i;  // Indexable data.
    int m_j;  // Indexable data.
    int m_k;  // Indexable data.
    CLogicalTimestamp m_lt;  // Timestamp.
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
        if( up == nullptr )
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
    virtual bool Commit( const CLogicalTimestamp& lt, int iActualCount ) = 0;

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
    virtual bool Commit( const CLogicalTimestamp& lt, int iActualCount ) override
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
    virtual CResult< CLogicalTimestamp > RetrieveData(
        const CLogicalTimestamp& ltLatestSeen,
        int iCount,
        OUT bool& bExhausted,
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
    CIXAvailability( Available available, const CLogicalTimestamp& ltLatestKnown ) :
        m_available( available ), m_ltLatestKnown( ltLatestKnown )
    {
    }

    // Default constructor.
    CIXAvailability() :
        m_available( Available::No )
    {
    }

    // Accesses the availability status.
    Available AccessAvailability() const { return m_available;  }

    // Accesses the latest known timestamp value.
    const CLogicalTimestamp& AccessLatestKnownTimestamp() const { return m_ltLatestKnown; }

private:
    Available m_available;  // Availability status.
    CLogicalTimestamp m_ltLatestKnown;  // Latest known timestamp.
};

// Data retrieval implementation.
class CIXDataRetrieval : public IIXDataRetrieval, public CLifeReporterAgent< CIXDataRetrieval >
{
public:

    // Constructor.
    CIXDataRetrieval()
        : m_iTotalCount( 0 ), m_iAcceptanceThreshold( 0 )
    {
        // Define the random number range.
        m_distr = std::uniform_int_distribution< int >( 1, 100 );
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
    virtual CResult< CLogicalTimestamp > RetrieveData(
        const CLogicalTimestamp& ltLatestSeen,
        int iCount,
        OUT bool& bExhausted,
        OUT vector< CIXItem >& vecItems
    ) override
    {
        // Reset out params.
        bExhausted = false;
        vecItems.clear();

        // Latest known timestamp after the initial threshold.
        CLogicalTimestamp ltLatestKnown = ltLatestSeen;

        // Determine the timestamp threshold.
        int iStart = ltLatestSeen.Get() + 1;

        // Retrieve the data.
        vecItems.reserve( iCount * 2 );
        int iItem = 0;
        for( iItem = iStart; iItem < iStart + iCount; iItem++ )
        {
            // Check the overall availability of data.
            if( m_iTotalCount >= GetGloballyAvailable() )
            {
                bExhausted = true;
                break;

            }  // end if

            // Store the data.
            ltLatestKnown = CLogicalTimestamp( iItem );
            if( m_distr( m_rd ) > m_iAcceptanceThreshold )
                vecItems.push_back( CIXItem( iItem * 2, iItem * 3, iItem * 4, ltLatestKnown ) );

            // Keep track of the count.
            m_iTotalCount++;

        }  // end for
        
        // Debug output.
        cout << Indent( 2 ) <<
                "Retrieved " <<
                vecItems.size() <<
                " items. Latest known timestamp is " <<
                ltLatestKnown.Get() <<
                "." <<
                endl;

        return CResult< CLogicalTimestamp >( true, ltLatestKnown );
    }

private:
    int m_iTotalCount;  // Total number of retrieved items.
    std::random_device m_rd;  // Obtain a random number from hardware.
	std::uniform_int_distribution< int > m_distr;  // Define the range.
    int m_iAcceptanceThreshold;  // Acceptance threshold.
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
    virtual const CLogicalTimestamp& AccessLatestSeen() const = 0;

    // Updates the locally stored timestamp if the provided one is later.
    virtual void UpdateIfLater( const CLogicalTimestamp& ltProvided ) = 0;

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
    CIXCallback( IIXDataRetrieval::SHP shpDataRetrieval, IIXIndexing::SHP shpIndexing, const CLogicalTimestamp& ltLatestSeen )
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
    virtual const CLogicalTimestamp& AccessLatestSeen() const override
    {
        // Access the timestamp.
        return m_ltLatestSeen;
    }

    // Updates the locally stored timestamp if the provided one is later.
    virtual void UpdateIfLater( const CLogicalTimestamp& ltProvided ) override
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
    CLogicalTimestamp m_ltLatestSeen;  // Latest seen timestamp.
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
    virtual CResult< CIXAvailability > MoveNext( const CLogicalTimestamp& ltLatestSeen ) = 0;
    
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
    virtual CResult< CIXAvailability > MoveNext( const CLogicalTimestamp& ltLatestSeen ) override
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
                    ? CIXAvailability( CIXAvailability::Available::Yes, m_ltLatestKnown )
                    : m_bExhausted
                            ? CIXAvailability( CIXAvailability::Available::No, m_ltLatestKnown )
                            : CIXAvailability( CIXAvailability::Available::Perhaps, m_ltLatestKnown );
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
        m_shpCB( shpCB ), m_bExhausted( false )
    {
        // Delegate.
        Reset( m_shpCB );
    }

    // Attempts to retrieve data to the local container.
    CIXAvailability RetrieveData( const CLogicalTimestamp& ltLatestSeen )
    {
        // Use the data source if available.
        _ASSERTE( m_shpCB );
        m_ltLatestKnown = ltLatestSeen;
        CIXAvailability retval( CIXAvailability::Available::No, m_ltLatestKnown );
        IIXDataRetrieval::SHP shpDataRetrieval = m_shpCB->AccessDataRetrieval();
        if( shpDataRetrieval )
        {
            // Retrieve the data and set the iterator.
            m_ltLatestKnown = IX_TRY( shpDataRetrieval->RetrieveData( ltLatestSeen, m_shpCB->GetChunkSize(),
                    OUT m_bExhausted, OUT m_vecItems ) );
            m_itr = m_vecItems.begin();

        }  // end if

        // Initialization status.
        m_bRetrieved = true;

        // Continuation status.
        if( m_ltLatestKnown.IsLaterThan( ltLatestSeen ) )
        {
            // Construct the availability result.
            retval = CIXAvailability( 
                    m_itr != m_vecItems.end()
                            ? CIXAvailability::Available::Yes 
                            : m_bExhausted
                                    ? CIXAvailability::Available::No
                                    : CIXAvailability::Available::Perhaps,
                    m_ltLatestKnown );
        }

        return retval;
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    bool m_bRetrieved;  // Indicates whether the data has been retrieved.
    CLogicalTimestamp m_ltLatestKnown;  // Latest known timestamp.
    bool m_bExhausted;  // Indicates whether the data source was exhausted.
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
    virtual CResult< CIXAvailability > MoveNext( const CLogicalTimestamp& ltLatestSeen_ ) override
    {
        // Locals.
        CResult< CIXAvailability > res( true, CIXAvailability() );
        CIXAvailability availability;
        CLogicalTimestamp ltLatestSeen = ltLatestSeen_;
        bool bContinueWithNewerTimestamp = true;

        // Loop while lower layers do not return a definitive Yes/No answer.
        while( bContinueWithNewerTimestamp )
        {
            // Reset the continuation flag.
            bContinueWithNewerTimestamp = false;

            // Proceed the enumerator.
            res = m_upLowerLayerEnum->MoveNext( ltLatestSeen );

            // Track the return value.
            availability = IX_TRY( res );
            switch( availability.AccessAvailability() )
            {

            // Data available.
            case CIXAvailability::Available::Yes:

                // Track the number of the received items.
                m_iCurrentCount++;
                break;

            // No data available.
            case CIXAvailability::Available::No:

                // Data source has been exhausted, so we need to commit the status.
                // TODO: FastForward?
                IX_TRY( Commit( ltLatestSeen ) );  // Return value ignored.
                break;

            // Perhaps data available.
            case CIXAvailability::Available::Perhaps:

                // There might be more data available, but in order to determine that we 
                // need to re-initialize the lower layers when we proceed the enumerator
                // next time. Before doing that, we must commit the current status if we
                // have received more items than the batch size.
                if( IsIntermediateCommitNeeded() )
                {
                    // Commit the current status.
                    IX_TRY( Commit( ltLatestSeen ) );  // Return value ignored.
                }
                else
                {
                    // Sanity check.
                    //***if( availability.AccessLatestKnownTimestamp().IsLaterThan( ltLatestSeen ) == false )
                    //***    return CResult< CIXAvailability >( false, res.AccessRetVal() );

                    // Forward the timestamp.
                    m_shpCB->UpdateIfLater( availability.AccessLatestKnownTimestamp() );  // void

                    // Reset the lower layers.
                    Reset( m_shpCB );  // void

                    // Use the updated timestamp.
                    ltLatestSeen = m_shpCB->AccessLatestSeen();

                    // Indicate that the loop should continue.
                    bContinueWithNewerTimestamp = true;

                }  // end if

                break;

            default:
                break;

            }  // end switch

        }  // end while

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
        m_iChunks++;
    }

private:

    // Delete the default constructor.
    CIXItemsBatched() = delete;

    // Constructor.
    CIXItemsBatched( IIXCallback::SHP shpCB ) :
        m_shpCB( shpCB ), m_iCurrentCount( 0 ), m_iChunks( 0 )
    {
        // Delegate.
        Reset( m_shpCB );  // void
    }

    // Commits the current progress.
    CResult< bool > Commit( const CLogicalTimestamp& lt )
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

    // Is intermediate commit needed?
    bool IsIntermediateCommitNeeded()
    {
        // Is the batch full?
        bool bBatchFull =
                m_iCurrentCount > 0 && m_iCurrentCount >= m_shpCB->GetBatchSize();

        // Do the chunks equal to a batch (even though we haven't seen them all)?
        bool bChunksEqualToBatch =
                m_iChunks * m_shpCB->GetChunkSize() >= m_shpCB->GetBatchSize();

        return bBatchFull || bChunksEqualToBatch;
    }

private:
    IIXCallback::SHP m_shpCB;  // Callback interface.
    IIXEnumerable::UP m_upLowerLayerEnum;  // The lower layer enumerator.
    int m_iChunks;  // The number of chunks used.
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
    virtual CResult< CIXAvailability > MoveNext( const CLogicalTimestamp& ltLatestSeen_ ) override
    {
        // Locals.
        CResult< CIXAvailability > res( true, CIXAvailability() );
        CIXAvailability availability;
        CLogicalTimestamp ltLatestSeen = ltLatestSeen_;
        bool bContinueWithNewerTimestamp = true;

        // Loop while lower layers do not return a definitive Yes/No answer.
        while( bContinueWithNewerTimestamp )
        {
            // Reset the continuation flag.
            bContinueWithNewerTimestamp = false;

            // Delegate to the lower layer enumerator.
            res = m_upLowerLayerEnum->MoveNext( ltLatestSeen );
            availability = IX_TRY( res );

            // Check the current availability first.
            if( availability.AccessAvailability() == CIXAvailability::Available::Perhaps )
            {
                // Sanity check.
                //***if( availability.AccessLatestKnownTimestamp().IsLaterThan( ltLatestSeen ) == false )
                //***    return CResult< CIXAvailability >( false, res.AccessRetVal() );

                // Forward the timestamp.
                m_shpCB->UpdateIfLater( availability.AccessLatestKnownTimestamp() );  // void

                // Reset the lower layers.
                Reset( m_shpCB );  // void

                // Use the updated timestamp.
                ltLatestSeen = m_shpCB->AccessLatestSeen();

                // Indicate that the looping should continue.
                bContinueWithNewerTimestamp = true;

            }  // end if

        }  // end while

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

// Helper aspect class for SearchEngine1 indexer jobs.
class CAIXJobSearchEngine1 : public CAIXJobBase
{
public:

    // Destructor.
    virtual ~CAIXJobSearchEngine1()
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

// Helper aspect class for SearchEngine2 indexer jobs.
class CAIXJobSearchEngine2 : public CAIXJobBase
{
public:

    // Destructor.
    virtual ~CAIXJobSearchEngine2()
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
    CLogicalTimestamp lt;

    // Callback.
    shared_ptr< IIXCallback > shpCB = shared_ptr< IIXCallback >( new CIXCallback( shpDataRetrieval, shpIndexing, lt ) );

    // Error handling.
    try
    {
        // Initialize the job.
        cout << "Job being created." << endl;
        typedef CIXJob< CAIXJobSearchEngine1 > CIXJOB;
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

