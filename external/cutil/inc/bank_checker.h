/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/* Helper to check for bank conflicts */

#ifndef _BANK_CHECKER_H_
#define _BANK_CHECKER_H_

// includes, system
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <limits>

//! Helper class to check for bank conflicts
class BankChecker 
{

    //! singleton, handle to the instance
    static BankChecker  bank_checker;

public:

    //! Get a handle to the instance of this class
    static BankChecker* const getHandle();

    //! Destructor
    ~BankChecker();

private:

    //! Constructor
    BankChecker();

public:

    //! Side effect of shared memory access
    //! @param tidx  thread id in x dimension of block
    //! @param tidy  thread id in y dimension of block
    //! @param tidz  thread id in z dimension of block
    //! @param bdimx block size in x dimension
    //! @param bdimy block size in y dimension
    //! @param bdimz block size in z dimension
    //! @param file  name of the source file where the access takes place
    //! @param line  line in the source file where the access takes place
    //! @param aname name of the array which is accessed
    //! @param index index into the array
    void access( unsigned int tidx, unsigned int tidy, unsigned int tidz,
                 unsigned int bdimx, unsigned int bdimy, unsigned int bdimz,
                 const char* file, const int line, const std::string& aname,
                 const int index);

private:

    //////////////////////////////////////////////////////////////////////////////
    //! Uniquely defined access location
    /////////////////////////////////////////////////////////////////////////////
    class AccessLocation {

        friend class BankChecker;

        //! type for hash value based on file + line + num_access
        typedef  std::string  KeyType;

    public:

        //! Constructor, default
        inline AccessLocation();

        //! Constructor
        //! @param file  file where the shared memory access takes place
        //! @param line  line in \a file where the shared memory access takes place
        //! @param array_name  name of the array which is accessed
        //! @param ltid  linearized thread id
        inline AccessLocation( const std::string& file, const unsigned int line,
                               const std::string& array_name, 
                               const unsigned int ltid );
    
        //! Constructor, copy
        inline AccessLocation( const AccessLocation&);
    
        //! Assignment operator
        inline AccessLocation& operator=( const AccessLocation&);

        //! Destructor
        ~AccessLocation();

    public:

        //! Get the hash key for the access based on file + line + num_access,
        //! i.e. identical accesses result in identical hash values
        inline const KeyType&  getKey() const;

    public:

        // operators, overloaded

        //! Equality
        inline bool operator==( const AccessLocation& other) const;

    public:

        //! Get the name of the file of the access
        const std::string&  getFile() const 
        {
            return file;
        }

        //! Get the line of the access in \a file
        const unsigned int&  getLine() const 
        {
            return line;
        }

        //! Get the number of the access in \a line
        const unsigned int&  getNumAccessLine() const 
        {
            return num_access_line;
        }

    private:

        // member variables

        //! file where shared mem is acccessed
        std::string  file;

        //! line where shared mem is accessed
        unsigned int line;

        //! number of access within line
        unsigned int num_access_line;

        //! name of the array which is accessed
        std::string array_name;

        //! pre-computed key value
        KeyType  key;

        // invalid flag for data field
        static const unsigned int invalid;

    private:
    
        // Counter for line accesses in a specific file
        // @key    file + line of access
        // @value  number of access in \a values::key
        typedef std::map<std::string, unsigned int>  LineAccessCounter;

        //! Tracker for number of accesses per file and line for each thread 
        //! The access number with the file+line uniquely identifies a specific 
        //! shared memory access
        //! Note: The access number itself would not be sufficient because there 
        //! might be branches in the code so that a specific access is only 
        //! by a subset of threads of a warp.
        //! @key  ltid (linearized thread id)
        //! @value line accesses in a specific file
        static  std::map< unsigned int, LineAccessCounter > thread_access;

    };

    //////////////////////////////////////////////////////////////////////////////
    //! Comparator for AccessLocation
    //////////////////////////////////////////////////////////////////////////////
    struct AccessLocationComparator {

        //! Compare operator for std container
        inline bool 
        operator()( const AccessLocation& lhs, const AccessLocation& rhs) const;
    };

private:

    //////////////////////////////////////////////////////////////////////////////
    //! Meta information about an access
    //////////////////////////////////////////////////////////////////////////////
    class AccessInfo {

        friend class BankChecker;

    public:

        //! Constructor, default
        inline AccessInfo();

        //! Constructor, default
        //! @param l  linearized thread id
        //! @param x  thread id in x dimension
        //! @param y  thread id in y dimension
        //! @param z  thread id in z dimension
        //! @param i  index used to access shared memory
        inline AccessInfo( const unsigned int l,
                           const unsigned int x, 
                           const unsigned int y,
                           const unsigned int z,
                           const unsigned int i );
    
        // Constructor, copy 
        inline AccessInfo( const AccessInfo& other);

        // Assignment operator
        inline AccessInfo& operator=( const AccessInfo& other);

        // Destructor
        ~AccessInfo();

    public:

        //! Provide infos as string
        const std::string getInfo() const;
        const unsigned int getIndex() const;
    private:

        //! linearized thread id
        unsigned int  ltid;

        // thread id in the three dimensions
        unsigned int  tidx;
        unsigned int  tidy;
        unsigned int  tidz;

        // index used to access shared memory
        unsigned int index;

        // invalid flag for data field
        static const unsigned int invalid;

    };

    private:

    // data members
  
    // list of access information
    typedef std::vector<AccessInfo>           AccessInfoList;
    typedef AccessInfoList::iterator          AccessInfoListIter;
    typedef AccessInfoList::const_iterator    AccessInfoListCIter;
  
    // information about the access to a specific location
    // @key    index  index of array access
    // @value  infos about threads which tried to access the threads
    typedef  std::map< unsigned int, AccessInfoList >  IndexAInfoList;
    typedef  IndexAInfoList::iterator                  IndexAInfoListIter;
    typedef  IndexAInfoList::const_iterator            IndexAInfoListCIter;
    
    typedef  std::map< AccessLocation,
                       IndexAInfoList, 
                       AccessLocationComparator >  AccessData;
    typedef  AccessData::iterator                  AccessDataIter;
    typedef  AccessData::const_iterator            AccessDataCIter;

    //! Tracker for shared memory accesses
    AccessData  access_data;

    //! Linearized thread id of last thread processed
    unsigned int  last_ltid;

    //! Total number of bank conflicts
    unsigned int total_num_conflicts;

    // flag if bank checker is used
    bool is_active;

private:

    // private helper functions
    
    //! Get the linearized thread id for a thread
    //! @return linear thread id
    //! @param tidx  thread id in x dimension of block
    //! @param tidy  thread id in y dimension of block
    //! @param tidz  thread id in z dimension of block
    //! @param bdimx block size in x dimension
    //! @param bdimy block size in y dimension
    //! @param bdimz block size in z dimension
    unsigned int 
    getLtid( unsigned int tidx, unsigned int tidy, unsigned int tidz,
             unsigned int bdimx, unsigned int bdimy, unsigned int bdimz );

    //! Reset all internal state for a new warp
    void  reset();

    //! Analyse the accesses by one warp
    void  analyse( const AccessDataCIter& iter_loc);

public:

    // constants

    // size of warp
    static const unsigned int warp_size = 16;

};

// functions, inlined

////////////////////////////////////////////////////////////////////////////////
// BankChecker::AccessLocation

////////////////////////////////////////////////////////////////////////////////
//! Constructor, standard
//! @param file  file where the shared memory access takes place
//! @param line  line in \a file where the shared memory access takes place
//! @param array_name  name of the array which is accessed
//! @param ltid  linearized thread id
////////////////////////////////////////////////////////////////////////////////
BankChecker::AccessLocation::
AccessLocation( const std::string& f, const unsigned int l,
                const std::string& aname,
                const unsigned int ltid ) :
    file( f),
    line( l),
    num_access_line( 0),
    array_name( aname)
{ 
    std::ostringstream oss;
    oss << file << "l" << line;
  
    const std::string sstr = oss.str();
    // check if this (file + line) has already been accessed by this thread
    if( thread_access[ltid].end() == thread_access[ltid].find( sstr)) {
        thread_access[ltid][oss.str()] = 0;
    }
    else {
        thread_access[ltid][oss.str()]++;
    }
  
    // number of access in this line by thread ltid
    num_access_line = thread_access[ltid][oss.str()];
  
    // generate key
    oss << "c" << num_access_line;
    key = oss.str();
}
   
////////////////////////////////////////////////////////////////////////////////
//! Constructor, copy
////////////////////////////////////////////////////////////////////////////////
BankChecker::AccessLocation::
AccessLocation( const AccessLocation& other) :
    file( other.file),
    line( other.line),
    num_access_line( other.num_access_line),
    array_name( other.array_name),
    key( other.key)
{ }

////////////////////////////////////////////////////////////////////////////////
//! Assignment operator
////////////////////////////////////////////////////////////////////////////////
BankChecker::AccessLocation& BankChecker::AccessLocation::
operator=( const AccessLocation& other) 
{

    if( this != &other) {

        file = other.file;
        line = other.line;
        array_name = other.array_name;
        num_access_line = other.num_access_line;
    }

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
//! Equality
////////////////////////////////////////////////////////////////////////////////
bool BankChecker::AccessLocation::
operator==( const AccessLocation& other) const 
{

    return (    (file == other.file) 
             && (line == other.line) 
             && (num_access_line == other.num_access_line)) ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
//! Get the hash key for the access based on file + line + num_access,
//! i.e. identical accesses result in identical hash values
//! @return the key value
////////////////////////////////////////////////////////////////////////////////
const BankChecker::AccessLocation::KeyType&  BankChecker::AccessLocation::
getKey() const {

    return key;
}

////////////////////////////////////////////////////////////////////////////////
//! Compare operator for std container
////////////////////////////////////////////////////////////////////////////////
inline bool 
BankChecker::AccessLocationComparator::operator()( const AccessLocation& lhs, 
                                              const AccessLocation& rhs) const {
    return (lhs.getKey() < rhs.getKey());
}

////////////////////////////////////////////////////////////////////////////////
// BankChecker::AccessInfo

////////////////////////////////////////////////////////////////////////////////
//! Constructor, default
//! @param l  linearized thread id
//! @param x  thread id in x dimension
//! @param y  thread id in y dimension
//! @param z  thread id in z dimension
////////////////////////////////////////////////////////////////////////////////
BankChecker::AccessInfo::
AccessInfo( const unsigned int l,
            const unsigned int x, 
            const unsigned int y,
            const unsigned int z,
            const unsigned int i ) :
    ltid( l),
    tidx( x),
    tidy( y),
    tidz( z),
    index( i)
{ }

////////////////////////////////////////////////////////////////////////////////
// Constructor, copy 
////////////////////////////////////////////////////////////////////////////////
BankChecker::AccessInfo::
AccessInfo( const AccessInfo& other) :
    ltid( other.ltid),
    tidx( other.tidx),
    tidy( other.tidy),
    tidz( other.tidz),
    index( other.index)
{ }

////////////////////////////////////////////////////////////////////////////////
// Assignment operator
////////////////////////////////////////////////////////////////////////////////
BankChecker::AccessInfo& BankChecker::AccessInfo::
operator=( const AccessInfo& other) 
{
  
    if( this != &other) 
    {

        ltid = other.ltid;
        tidx = other.tidx;
        tidy = other.tidy;
        tidz = other.tidz;
        index = other.index;
    }

  return *this;
}

#endif // #ifdef _BANK_CHECKER_H_
