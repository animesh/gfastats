//
//  gfastats-sak.h
//
//
//  Created by Giulio Formenti on 1/30/22.
//

#ifndef GFASTATS_SAK_H
#define GFASTATS_SAK_H

// unordered map to handle out correspondence in following switch statement
const static phmap::flat_hash_map<std::string,int> string_to_case{
    {"JOIN", 1},
    {"SPLIT", 2},
    {"EXCISE", 3},
    {"REMOVE", 4},
    {"ERASE", 5},
    {"RVCP", 6},
    {"INVERT", 7}
};

struct Instruction {
    
    std::string action;
    std::string path1;
    std::string path2;
    std::string path3;
    std::string contig1;
    std::string contig2;
    std::string comment1;
    std::string comment2;
    std::string gHeader = "";
    
    char pId1Or, pId2Or, sId1Or, sId2Or;
    
    unsigned int gUId = 0, dist = 0, start1 = 0, end1 = 0, start2 = 0, end2 = 0;
    
};

class SAK { // the swiss army knife
private:
    InSequences inSequences;
    InSegment inSegment1, inSegment2, inSegmentNew;
    std::string sId1Header, sId2Header;
    
public:
    
    Instruction readInstruction(std::string line) {
        
        std::string delimiter = "\t";
        std::vector<std::string> arguments;
        
        size_t pos = 0, pos2 = 0, pos3 = 0;
        
        while ((pos = line.find(delimiter)) != std::string::npos) {
            
            arguments.push_back(line.substr(0, pos));
            
            line.erase(0, pos + delimiter.length());
        
        }
        
        arguments.push_back(line); // last column
        
        Instruction instruction;
        
        for (auto & c: arguments[0]) instruction.action += (char) toupper(c);
        
        switch (string_to_case.count(instruction.action) ? string_to_case.at(instruction.action) : 0) {
            
            case 1: { // JOIN
                
                instruction.pId1Or = arguments[1].back(); // get sequence orientation in the gap
                
                arguments[1].pop_back(); // remove sequence orientation in the gap
                
                if(arguments[1].back() == ')') {
                    
                    pos = arguments[1].find('(');
                    pos2 = arguments[1].find(':');
                    pos3 = arguments[1].find(')');
                    
                    instruction.path1 = arguments[1].substr(0, pos);
                    instruction.start1 = stoi(arguments[1].substr(pos + 1, pos2 - pos - 1));
                    instruction.end1 = stoi(arguments[1].substr(pos2 + 1, pos3 - pos2 - 1));
                    
                }else{
                
                    instruction.path1 = arguments[1];
                
                }
                    
                instruction.pId2Or = arguments[2].back(); // get sequence orientation in the gap
                
                arguments[2].pop_back(); // remove sequence orientation in the gap
                
                if(arguments[2].back() == ')') {
                    
                    pos = arguments[2].find('(');
                    pos2 = arguments[2].find(':');
                    pos3 = arguments[2].find(')');
                    
                    instruction.path2 = arguments[2].substr(0, pos);
                    instruction.start2 = stoi(arguments[2].substr(pos + 1, pos2 - pos - 1));
                    instruction.end2 = stoi(arguments[2].substr(pos2 + 1, pos3 - pos2 - 1));
                    
                }else{
                
                    instruction.path2 = arguments[2];
                
                }
                
                instruction.dist = stoi(arguments[3]);
                
                instruction.gHeader = arguments[4];
                
                instruction.path3 = arguments[5];
                
                if (arguments[6] != "") {
                
                    instruction.gUId = stoi(arguments[6]);
                    
                }
                
                break;
            }
                
            case 2: { // SPLIT
                
                instruction.contig1 = arguments[1];
                
                instruction.contig2 = arguments[2];
                
                instruction.path1 = arguments[3];
                
                instruction.path2 = arguments[4];
                
                instruction.comment1 = arguments[5];
                
                instruction.comment2 = arguments[6];
                
                break;
            }

            case 3: { // EXCISE
                
                instruction.contig1 = arguments[1];
                
                if (arguments[2] != "") {
                
                    instruction.dist = stoi(arguments[2]);
                    
                }else{
                    
                    instruction.dist = 0;
                    
                }
                
                instruction.gHeader = arguments[3];
                
                break;
            }
                
            case 4: { // REMOVE
                
                instruction.contig1 = arguments[1];
                
                break;
            }
                
            case 5: { // ERASE
                
                size_t pos1 = 0, pos2 = 0;
                
                pos1 = arguments[1].find(":");
                
                pos2 = arguments[1].find("-");
                
                instruction.contig1 = arguments[1].substr(0, pos1);
                
                instruction.start1 = stoi(arguments[1].substr(pos1+1, pos2));
                
                instruction.end1 = stoi(arguments[1].substr(pos2+1, arguments[1].size()+1));
                
                break;
            }
                
            case 6: { // RVCP
                
                instruction.contig1 = arguments[1];
                
                break;
            }
                
            case 7: { // INVERT
                
                instruction.contig1 = arguments[1];
                
                break;
            }
            
            default:
                fprintf(stderr, "unrecognized action %s\n", instruction.action.c_str());
                exit(1);
        }
        
        verbose("Instruction read");
        
        return instruction;
        
    }
    
    bool executeInstruction(InSequences& inSequences, Instruction instruction) {
        
        switch (string_to_case.count(instruction.action) ? string_to_case.at(instruction.action) : 0) {
            case 1: { // JOIN
                
                join(inSequences, instruction);
                
                break;
                
            }
                
            case 2: { // SPLIT
                
                split(inSequences, instruction);
                
                break;
                
            }
                
            case 3: { // EXCISE
                
                excise(inSequences, instruction);
                
                break;
                
            }

            case 4: { // REMOVE
                
                remove(inSequences, instruction);
                
                break;
                
            }
                
            case 5: { // ERASE
                
                erase(inSequences, instruction);
                
                break;
                
            }
                
            case 6: { // RVCP
                
                rvcp(inSequences, instruction);
                
                break;
                
            }
                
            case 7: { // INVERT
                
                invert(inSequences, instruction);
                
                break;
                
            }
                
            default:
                fprintf(stderr, "unrecognized action %s\n", instruction.action.c_str());
                return EXIT_FAILURE;
        }
        
        return true;
        
    }
    
    bool join(InSequences& inSequences, Instruction instruction) { // joins two sequences via a gap based on instruction
        
        inSequences.joinPaths(instruction.path3, inSequences.headersToIds[instruction.path1], inSequences.headersToIds[instruction.path2], instruction.gHeader, instruction.gUId, instruction.pId1Or, instruction.pId2Or, instruction.dist, instruction.start1, instruction.end1, instruction.start2, instruction.end2); // generate a new path by joining the paths that contain the two segments
        
        return true;
        
    }
    
    bool split(InSequences& inSequences, Instruction instruction) { // splits two sequences removing the gap in between based on instruction
        
        std::vector<unsigned int> guIds = inSequences.removeGaps(&instruction.contig1, &instruction.contig2); // remove the gap
        
        inSequences.splitPath(guIds[0], instruction.path1, instruction.path2); // generate two new the paths splitting the original path
        
        return true;
        
    }

    bool excise(InSequences& inSequences, Instruction instruction) { // excises a sequence, removing also edges if present and optionally adding a gap

        std::vector<InGap> oldGaps = inSequences.getGap(&instruction.contig1); // get the gaps associated with contig1
            
        InGap gap;
        
        inSequences.uId++;
    
        gap.newGap(inSequences.uId, oldGaps[0].getsId1(), oldGaps[1].getsId2(), oldGaps[0].getsId1Or(), oldGaps[1].getsId2Or(), instruction.dist, instruction.gHeader); // define the new gap
        
        inSequences.insertHash1(instruction.gHeader, inSequences.uId); // header to hash table
        inSequences.insertHash2(inSequences.uId, instruction.gHeader); // header to hash table
        
        inSequences.addGap(gap); // introduce the new gap
        
        inSequences.removeGaps(&instruction.contig1);
            
        inSequences.removeSegmentInPath(inSequences.headersToIds[instruction.contig1], gap); // removes the segment from the path
        
        InPath path;
        
        path.setHeader(instruction.contig1);
        
        path.add('S', inSequences.headersToIds[instruction.contig1], '+');
        
        inSequences.addPath(path);
            
        return true;
        
    }
    
    bool remove(InSequences& inSequences, Instruction instruction) { // removes a segment
        
        inSequences.removePathsFromSegment(inSequences.headersToIds[instruction.contig1]); // remove the paths involving contig1
        
        inSequences.removeSegment(&instruction.contig1); // remove the segment
        
        inSequences.updateGapLens();
        
        return true;
        
    }
    
    bool erase(InSequences& inSequences, Instruction instruction) { // erases a portion of sequence
        
        inSequences.inSegments[inSequences.headersToIds[instruction.contig1]].trimSegment(instruction.start1, instruction.end1); // trim segment
        
        inSequences.changeTotSegmentLen(instruction.start1-instruction.end1);
        
        return true;
        
    }
    
    bool rvcp(InSequences& inSequences, Instruction instruction) { // reverse complement sequence
        
        unsigned int uId = inSequences.headersToIds[instruction.contig1], sIdx = 0;
        
        auto sId = find_if(inSequences.inSegments.begin(), inSequences.inSegments.end(), [uId](InSegment& obj) {return obj.getuId() == uId;}); // given a node uId, find it
    
        if (sId != inSequences.inSegments.end()) {sIdx = std::distance(inSequences.inSegments.begin(), sId);} // gives us the segment index
        
        inSequences.inSegments[sIdx].rvcpSegment(); // rvcp segment
        
        return true;
        
    }
    
    bool invert(InSequences& inSequences, Instruction instruction) { // invert sequence
        
        unsigned int uId = inSequences.headersToIds[instruction.contig1], sIdx = 0;
        
        auto sId = find_if(inSequences.inSegments.begin(), inSequences.inSegments.end(), [uId](InSegment& obj) {return obj.getuId() == uId;}); // given a node uId, find it
    
        if (sId != inSequences.inSegments.end()) {sIdx = std::distance(inSequences.inSegments.begin(), sId);} // gives us the segment index
        
        inSequences.inSegments[sIdx].invertSegment(); // invert segment
        
        return true;
        
    }
    
};

#endif /* GFASTATS_SAK_H */
