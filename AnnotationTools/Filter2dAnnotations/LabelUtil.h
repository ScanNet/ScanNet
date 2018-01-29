#pragma once

class LabelUtil
{
public:
	LabelUtil() {}
	void init(const std::string& labelMapFile, const std::string& labelName = "category", const std::string& idName = "") {
		if (!util::fileExists(labelMapFile)) throw MLIB_EXCEPTION(labelMapFile + " does not exist!");
		m_maxLabel = 65535;//255;

		getLabelMappingFromFile(labelMapFile, labelName, idName);

		m_bIsInitialized = true;
	}

	static LabelUtil& getInstance() {
		static LabelUtil s;
		return s;
	}
	static LabelUtil& get() {
		return getInstance();
	}

	bool getIdForLabel(const std::string& label, unsigned short& id) const {
		const auto it = s_labelsToIds.find(label);
		if (it == s_labelsToIds.end()) return false;
		id = it->second; 
		return true;
	}

	bool getLabelForId(unsigned short id, std::string& label) const {
		const auto it = s_idsToLabels.find(id);
		if (it == s_idsToLabels.end()) return false;
		label = it->second;
		return true;
	}

private:

	void getLabelMappingFromFile(const std::string& filename, const std::string& labelName, const std::string& idName)
	{
		if (!util::fileExists(filename)) throw MLIB_EXCEPTION("label mapping files does not exist!");
		const char splitter = '\t';
		
		s_labelsToIds.clear();
		s_idsToLabels.clear();

		std::ifstream s(filename); std::string line;
		//read header
		std::unordered_map<std::string, unsigned int> header;
		if (!std::getline(s, line)) throw MLIB_EXCEPTION("error reading label mapping file");
		auto parts = util::split(line, splitter);
		const unsigned int numElems = (unsigned int)parts.size();
		for (unsigned int i = 0; i < parts.size(); i++) header[parts[i]] = i;

		auto it = header.find(labelName);
		if (it == header.end()) throw MLIB_EXCEPTION("could not find value " + labelName + " in label mapping file");
		unsigned int labelIdx = it->second;
		bool bUseExistingLabel = !idName.empty(); unsigned int idIdx = (unsigned int)-1;
		if (bUseExistingLabel) {
			it = header.find(idName);
			if (it == header.end()) throw MLIB_EXCEPTION("could not find value " + idName + " in label mapping file");
			idIdx = it->second;
		}
		//read elements
		unsigned int lineCount = 1;
		while (std::getline(s, line)) {
			parts = util::split(line, splitter, true);
			if (!parts[labelIdx].empty() && (!bUseExistingLabel || !parts[idIdx].empty())) {
				unsigned int id = bUseExistingLabel ? util::convertTo<unsigned int>(parts[idIdx]) : lineCount;
				if (id > m_maxLabel) //skip
					continue;
				s_labelsToIds[parts[labelIdx]] = (unsigned short)id;
				s_idsToLabels[(unsigned short)id] = parts[labelIdx];
			}
			++lineCount;
		}
		s.close();

		std::cout << "read " << s_labelsToIds.size() << " labels" << std::endl;
	}

	bool			m_bIsInitialized;
	unsigned short	m_maxLabel;

	std::unordered_map<std::string, unsigned short> s_labelsToIds;
	std::unordered_map<unsigned short, std::string> s_idsToLabels;

};