//
//  gfastats-input.h
//  gfastats
//
//  Created by Giulio Formenti on 1/16/22.
//

#ifndef gfastats_input_h
#define gfastats_input_h

class InFile {
    
    std::string h;
    char* c;
    std::vector<std::string> bedIncludeListHeaders;
    std::vector<std::string> bedExcludeListHeaders;
    unsigned int pos = 0, cBegin = 0, cEnd = 0, offset = 0, prevCEnd = 0;
    
public:
    
    InSequences readFiles(std::string &iSeqFileArg, std::string &iBedIncludeFileArg, std::string &iBedExcludeFileArg, BedCoordinates &bedIncludeList, bool isPipe, char &pipeType) {
        
        std::string newLine, seqHeader, seqComment, inSequence, inSequenceQuality, line, bedHeader;
        
        std::unique_ptr<std::istream> stream;
        
        unsigned int idx = 0, begin = 0, end = 0;
        
        if (!iBedIncludeFileArg.empty() || (isPipe && (pipeType == 'i'))) {
            
            if (isPipe && (pipeType == 'i')) {
                
                stream = make_unique<std::istream>(std::cin.rdbuf());
                
            }else{
                
                stream = make_unique<std::ifstream>(std::ifstream(iBedIncludeFileArg));
                
            }
            
            while (getline(*stream, line)) {
                
                std::istringstream iss(line);
                iss >> bedHeader >> begin >> end;
                
                bedIncludeList.pushCoordinates(bedHeader, begin, end);
                begin = 0, end = 0;
                
            }
            
        }
        
        BedCoordinates bedExcludeList;
        
        if (!iBedExcludeFileArg.empty() || (isPipe && (pipeType == 'e'))) {
            
            if (isPipe && (pipeType == 'e')) {
                
                stream = make_unique<std::istream>(std::cin.rdbuf());
                
            }else{
                
                stream = make_unique<std::ifstream>(std::ifstream(iBedExcludeFileArg));
                
            }
            
            while (getline(*stream, line)) {
                
                std::istringstream iss(line);
                iss >> bedHeader >> begin >> end;
                
                bedExcludeList.pushCoordinates(bedHeader, begin, end);
                begin = 0, end = 0;
                
            }
            
        }
        
        InSequences inSequences;
        
        std::string firstLine;
        char firstChar;
        unsigned char buffer[2];
        
        std::ifstream is(iSeqFileArg);
        
        // this stream takes input from a gzip compressed file
        zstream::igzstream zfin(is);
        
        if (determineGzip(iSeqFileArg)) { // input is a compressed file
            
            stream = make_unique<std::istream>(zfin.rdbuf());
            
        } else if (isPipe && (pipeType == 's')) { // input is from pipe
            
            std::cin.read((char*)(&buffer[0]), 2);

            if (buffer[0] == 0x1f && (buffer[1] == 0x8b)) { // check if pipe is gzipped

                // this stream takes input from a gzip compressed pipe
                zstream::igzstream zin(std::cin);
                
                stream = make_unique<std::istream>(zin.rdbuf());
                
                getline(*stream, newLine);
                
                std::cout<<"Gz pipe currently not supported\n";
                
                exit(1);

            }else{

                stream = make_unique<std::istream>(std::cin.rdbuf());

            }

        } else { // input is a regular plain text file

            stream = make_unique<std::ifstream>(std::ifstream(iSeqFileArg));

        }
        
        if (stream) {
            
            getline(*stream, newLine);
            
            if (isPipe && (pipeType == 's')) { // if input from pipe, recover the first two bytes
                
                newLine.insert (0, 1, buffer[1]);
                newLine.insert (0, 1, buffer[0]);
                
            }
            
            firstLine = newLine;
            firstChar = newLine[0];
            
            if ((!isPipe || pipeType != 's') && !determineGzip(iSeqFileArg)) {
                
                stream->clear();
                stream->seekg(0, stream->beg);
                
            }
            
            switch (firstChar) {
                    
                case '>': {
                    
                    if ((isPipe && pipeType == 's') || determineGzip(iSeqFileArg)) {
                        
                        parseFasta(firstLine, inSequences, seqHeader, seqComment, inSequence, idx, bedIncludeList, bedExcludeList);
                        
                    }
                    
                    while (getline(*stream, newLine)) {
                        
                        parseFasta(newLine, inSequences, seqHeader, seqComment, inSequence, idx, bedIncludeList, bedExcludeList);
                        
                    }
                    
                    includeExcludeAppend(&inSequences, &seqHeader, &seqComment, &inSequence, bedIncludeList, bedExcludeList);
                    
                    verbose(verbose_flag, "End of file");
                    
                    break;
                }
                case '@': {
                    
                    if (isPipe && pipeType == 's') { // pipe input
                        
                        firstLine.erase(0, 1);
                        
                        h = std::string(strtok(strdup(firstLine.c_str())," ")); //process header line
                        c = strtok(NULL,""); //read comment
                        
                        seqHeader = h;
                        
                        if (c != NULL) {
                            
                            seqComment = std::string(c);
                            
                        }
                        
                        getline(*stream, newLine);
                        inSequence = newLine;
                        
                        getline(*stream, newLine);
                        
                        getline(*stream, newLine);
                        inSequenceQuality = newLine;
                        
                        includeExcludeAppend(&inSequences, &seqHeader, &seqComment, &inSequence, bedIncludeList, bedExcludeList, &inSequenceQuality);
                        
                    }
                    
                    while (getline(*stream, newLine)) { // file input
                        
                        newLine.erase(0, 1);
                        
                        h = std::string(strtok(strdup(newLine.c_str())," ")); //process header line
                        c = strtok(NULL,""); //read comment
                        
                        seqHeader = h;
                        
                        if (c != NULL) {
                            
                            seqComment = std::string(c);
                            
                        }
                        
                        getline(*stream, newLine);
                        inSequence = newLine;
                        
                        getline(*stream, newLine);
                        
                        getline(*stream, newLine);
                        inSequenceQuality = newLine;

                        includeExcludeAppend(&inSequences, &seqHeader, &seqComment, &inSequence, bedIncludeList, bedExcludeList, &inSequenceQuality);
                        
                    }
                    
                    break;
                    
                }
                default: {
                    
                    std::string h_col1, h_col2, h_col3, s, version;
                    char* v;
                    
                    InGap inGap;
                    
                    unsigned long long int lineN = 1;
                    unsigned long long int seqN = 0;
                    
                    h_col1 = std::string(strtok(strdup(firstLine.c_str()),"\t")); // process first line
                    
                    if (h_col1 == "H") {
                        
                        h_col2 = strtok(NULL,""); // read header col2
                        std::string(strtok(strdup(h_col2.c_str()),":")); // process version tag
                        strtok(NULL,":");
                        v = strtok(NULL,"");
                        
                        if (v != NULL) {
                            
                            version = std::string(v);
                            verbose(verbose_flag, "GFA version: " + version);
                            
                        }else{
                            
                            printf("Cannot recognize GFA version");
                            printf("Offending line: %s", firstLine.c_str());
                            printf("Trying to detect from content");
                            
                            if (h_col1 == "S") {
                                
                                h_col3 = strtok(NULL,""); // read sequence col3
                                
                                
                                if (isInt(h_col3) || h_col3 == "*") {
                                    
                                    version = '2';
                                    verbose(verbose_flag, "Proposed GFA version: " + version);
                                    
                                }else{
                                    
                                    version = '1';
                                    verbose(verbose_flag, "Proposed GFA version: " + version);
                                    
                                }
                                
                                
                            }else if (h_col1 == "G") {
                                
                                version = '2';
                                verbose(verbose_flag, "Proposed GFA version: " + version);
                                
                            }
                            
                        }
                        
                    }
                    
                    if (version[0] == '2') {
                    
                        while (getline(*stream, newLine)) {
                            
                            switch (newLine[0]) {
                                    
                                case 'S': {
                                    
                                    strtok(strdup(newLine.c_str()),"\t"); //process first line
                                    h = strtok(NULL,"\t");
                                    
                                    seqHeader = h;
                                    
                                    strtok(NULL,"\t");
                                    s = strtok(NULL,"\t");
                                    inSequence = s;
                                    
                                    c = strtok(NULL,"\t");
                                    if (c != NULL) {
                                        
                                        seqComment = std::string(c);
                                        
                                    }
                                    
                                    inSequences.insertHash(seqHeader, seqN); // header to hash table
                                    
                                    includeExcludeAppend(&inSequences, &seqHeader, &seqComment, &inSequence, bedIncludeList, bedExcludeList);
                                    seqN++;
                                    lineN++;
                                    
                                    break;
                                    
                                }
                                case 'G': {
                                    
                                    inGap.readLine(&newLine, &lineN);
                                    
                                    inSequences.appendGap(inGap);
                                    
                                    lineN++;
                                                 
                                    break;
                                    
                                }
                                    
                            }
                            
                        }
                        
                        inSequences.headersTosIds();  // assign sIds to all headers in gaps
                        
                    }else if (version[0] == '1') {
                    
                        while (getline(*stream, newLine)) {
                            
                            switch (newLine[0]) {
                                    
                                case 'S': {
                                    
                                    strtok(strdup(newLine.c_str()),"\t"); //process first line
                                    h = strtok(NULL,"\t");
                                    
                                    seqHeader = h;
                                    
                                    s = strtok(NULL,"\t");
                                    inSequence = s;
                                    
                                    c = strtok(NULL,"\t");
                                    if (c != NULL) {
                                        
                                        seqComment = std::string(c);
                                        
                                    }
                                    
                                    inSequences.insertHash(seqHeader, seqN); // header to hash table
                                    
                                    includeExcludeAppend(&inSequences, &seqHeader, &seqComment, &inSequence, bedIncludeList, bedExcludeList);
                                    seqN++;
                                    lineN++;
                                    
                                    break;
                                    
                                }
                                case 'G': {
                                    
                                    inGap.readLine(&newLine, &lineN);
                                    
                                    inSequences.appendGap(inGap);
                                    
                                    lineN++;
                                    
                                    break;
                                    
                                }
                                    
                            }
                            
                        }
                        
                        break;
                        
                    }
                    
                }
                
            }
            
            inSequences.updateStats();
            
        }else{

            printf("Stream not successful: %s", iSeqFileArg.c_str());

        }
        
        return inSequences;
        
    }
    
    void parseFasta(std::string &newLine, InSequences &inSequences, std::string &seqHeader, std::string &seqComment, std::string &inSequence, unsigned int &idx, BedCoordinates bedIncludeList, BedCoordinates bedExcludeList) {
        
        switch (newLine[0]) {
            case '\n':{
                
                break;
            
            }
            case '>': {
                
                if (idx> 0) {
                    
                    verbose(verbose_flag, "Individual fasta sequence read");
                    
                    includeExcludeAppend(&inSequences, &seqHeader, &seqComment, &inSequence, bedIncludeList, bedExcludeList);
                    
                    inSequence = "";
                    
                }
                
                newLine.erase(0, 1);
                
                h = std::string(strtok(strdup(newLine.c_str())," ")); //process header line
                c = strtok(NULL,""); //read comment
                
                seqHeader = h;
                
                if (c != NULL) {
                    
                    seqComment = std::string(c);
                    
                }
                
                idx++;
                
                break;
            }
            case ' ':{
                
                break;
            }
            default: {
                
                inSequence.append(newLine);
                
            }
                
        }
        
    }
    
    void includeExcludeAppend(InSequences* inSequences, std::string* seqHeader, std::string* seqComment, std::string* inSequence, BedCoordinates bedIncludeList, BedCoordinates bedExcludeList, std::string* inSequenceQuality = NULL) {
 
        bedIncludeListHeaders = bedIncludeList.getSeqHeaders();
        bedExcludeListHeaders = bedExcludeList.getSeqHeaders();
        bool outSeq;
        
        if   (bedIncludeList.empty() &&
              bedExcludeList.empty()) {
            
            inSequences->appendSequence(seqHeader, seqComment, inSequence, inSequenceQuality);
            
        }else if
            (!bedIncludeList.empty() &&
             bedExcludeList.empty()) {
            
            offset = 0, prevCEnd = 0;
            outSeq = false;
            
            auto it = begin(bedIncludeListHeaders);
            
            while (it != end(bedIncludeListHeaders)) {
                
                it = std::find(it, bedIncludeListHeaders.end(), *seqHeader);
                
                if (it == end(bedIncludeListHeaders) || bedIncludeList.getSeqHeader(pos) != *seqHeader) {
                    
                    break;
                    
                }
                
                outSeq = true;

                cBegin = bedIncludeList.getcBegin(pos);
                cEnd = bedIncludeList.getcEnd(pos);
                
                if (!(cBegin == 0 && cEnd == 0)) {
                    
                    inSequence->erase(offset, cBegin-prevCEnd);
                    
                    if (inSequenceQuality != NULL) {
                    
                        inSequenceQuality->erase(offset, cBegin-prevCEnd);
                    
                    }
                        
                    offset += cEnd-cBegin;
                    prevCEnd = cEnd;
                    
                }
              
                ++it;
                pos++;
                
            }
                
            if (outSeq && inSequence->size()>0) {
                
                if (offset>0) {
                
                    inSequence->erase(offset, inSequence->size()-offset);
                    
                    if (inSequenceQuality != NULL) {
                    
                        inSequenceQuality->erase(offset, inSequenceQuality->size()-offset);
                        
                    }
                    
                }
                
                inSequences->appendSequence(seqHeader, seqComment, inSequence, inSequenceQuality);
            
            }else {
                
                verbose(verbose_flag, "Scaffold entirely removed as a result of include: " + *seqHeader);
                
            }
                
        }else if
            (bedIncludeList.empty() &&
             !bedExcludeList.empty()) {
                
            offset = 0;
            outSeq = true;
            
            auto it = begin(bedExcludeListHeaders);
            
            while (it != end(bedExcludeListHeaders)) {
                
                it = std::find(it, bedExcludeListHeaders.end(), *seqHeader);
                
                if (it == end(bedExcludeListHeaders) || bedExcludeList.getSeqHeader(pos) != *seqHeader) {
                    
                    break;
                    
                }

                cBegin = bedExcludeList.getcBegin(pos);
                cEnd = bedExcludeList.getcEnd(pos);
                
                if (!(cBegin == 0 && cEnd == 0)) {
                    
                    inSequence->erase(cBegin-offset, cEnd-cBegin);
                    
                    if (inSequenceQuality != NULL) {
                    
                        inSequenceQuality->erase(cBegin-offset, cEnd-cBegin);
                        
                    }
                        
                    offset += cEnd-cBegin;
                    
                }else{
                    
                    outSeq = false;
                    
                }
              
                ++it;
                pos++;
                
            }
                
            if (outSeq && inSequence->size()>0) {
            
                inSequences->appendSequence(seqHeader, seqComment, inSequence, inSequenceQuality);
            
            }else {
                
                verbose(verbose_flag, "Scaffold entirely removed as a result of exclude: " + *seqHeader);
                
            }
                    
        }else if
                (!bedIncludeList.empty() &&
                 !bedExcludeList.empty() &&
                 std::find(bedIncludeListHeaders.begin(), bedIncludeListHeaders.end(), *seqHeader) != bedIncludeListHeaders.end() &&
                 std::find(bedExcludeListHeaders.begin(), bedExcludeListHeaders.end(), *seqHeader) == bedExcludeListHeaders.end()) {
                    
                    inSequences->appendSequence(seqHeader, seqComment, inSequence, inSequenceQuality);
                    
        }
        
    }
    
};

#endif /* gfastats_input_h */
