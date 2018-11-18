#pragma once


#include "mLibInclude.h"

#define X_PROCESSED_FILE_STATE_FIELDS \
	X(bool, valid) \
	X(unsigned int, heapFreeCount) \
	X(unsigned int, numValidOptTransforms) \
	X(unsigned int, numTransforms) \
	X(bool, aligned)


#ifndef VAR_NAME
#define VAR_NAME(x) #x
#endif

#define checkSizeArray(a, d)( (((sizeof a)/(sizeof a[0])) >= d))

class ProcessedFile
{
public:

#define X(type, name) type name;
	X_PROCESSED_FILE_STATE_FIELDS
#undef X

		//! sets the parameter file and reads
		void readMembers(const ParameterFile& parameterFile) {
		m_ParameterFile = parameterFile;
		readMembers();
	}

	//! reads all the members from the given parameter file (could be called for reloading)
	void readMembers() {
		aligned = false;

#define X(type, name) \
	if (!m_ParameterFile.readParameter(std::string(#name), name) && std::string(#name) != "aligned") {MLIB_WARNING(std::string(#name).append(" ").append("uninitialized"));	name = type();}
		X_PROCESSED_FILE_STATE_FIELDS
#undef X
		
		m_bIsInitialized = true;
	}

	template<class T>
	std::string makeString(const T& in) {
		std::string ret = std::to_string(in);
		return ret;
	}
	template <>
	std::string makeString<bool>(const bool& in) {
		if (in == true) return "true";
		else return "false";
	}

	void saveToFile(const std::string& outFile) {
		std::ofstream out(outFile);
#define X(type, name) \
			{ out << #name << " = " << makeString(name) << std::endl; }
		X_PROCESSED_FILE_STATE_FIELDS
#undef X
	}

	void print() const {
#define X(type, name) \
	std::cout << #name " = " << name << std::endl;
		X_PROCESSED_FILE_STATE_FIELDS
#undef X
	}


	//! constructor
	ProcessedFile() {
		m_bIsInitialized = false;
	}

	//! destructor
	~ProcessedFile() {
	}
	

private:
	bool			m_bIsInitialized;
	ParameterFile	m_ParameterFile;
};

