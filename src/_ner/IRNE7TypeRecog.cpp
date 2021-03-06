#include "IRNE7TypeRecog.h"
#include <math.h>


//IRNErecog::IRNErecog(string& modelName)
//{
//	readTemplateFile();
	//initRuleMaps();
	
//	string model = "DATA//" + modelName;
//	MEmodel.load(model);
	
	/*temp.open("outNE.txt");
    tempProb.open("outProb.txt");
	srcProb.open("srcProb.txt");*/
//}

IRNErecog::IRNErecog()
{

}
IRNErecog::~IRNErecog()
{
	
}


void IRNErecog::setObject(InitDic *pdic, CModel* model)
{
	this->Cmodel = model;
	this->pdic = pdic;
	ruletest.setObject(pdic);
}

void IRNErecog::initRuleMaps()
{
	string filename[5] = {"Nitypeend.txt", "Nstypeend.txt", "Nztypeend.txt", "preNh.txt", "sufNh.txt"};
	ifstream file[5];
	string Word;
	for (int i=0; i<5; ++i)
	{
		file[i].open(("DATA//"+filename[i]).c_str());
		if (!file[i])
		{ 
			cerr << "Can not open the " << filename[i] << endl;
			exit(-1);
		}

		while (getline(file[i], Word))
		{		
			switch (i)
			{
			case 0: map_Niend[Word] = 1;
				break;
			case 1: map_Nsend[Word] = 2;
				break;
			case 2: map_Nzend[Word] = 3;
				break;
			case 3: map_preNh[Word] = 4;
				break;
			case 4: map_sufNh[Word] = 5;
				break;
			default:break;
			}
		}
		file[i].close();
	}
}
/*
 *	It is the interface for NE recognize. The parameter strSen is the input sentense
 *  that will deal with, and we will write the result into the outfile. 
 */
void IRNErecog::IRNE7TypeRecog(const string& strSen, string& StrOut, int tagForm, bool* isNEtypeFlag)
{
	if (!(isNEtypeFlag[0] || isNEtypeFlag[1] || isNEtypeFlag[2]))
	{
		return;
	}
	else
	{
		bIsNEtypeFlag = isNEtypeFlag;
	}
    //string NEResult;
	vec2paSen.clear();
	vecList.clear();
	splitSenByWord(strSen, vec2paSen, ' ');
	//showvec2pairContent(vec2paSen);
	vecList.resize(vec2paSen.size());

	for (int i=0; i<(int)vec2paSen.size(); i++)
	{
		//temp << "word" << i << ": " << endl;
		NErecogAtCurrentPostion(i);
	}
	getBestNEResult();
	//cutSingleNE(vecNEResult);

	ruletest.RuleNErecog(vecNEResult, vec2paSen);

	getNEResult(StrOut, tagForm);
	
	//outfile << NEResult << endl;
    //outvecList(vecList, temp);
}

/*
 *	function:
 */
void IRNErecog::NErecogAtCurrentPostion(int position)
{
	string feature;
	
	int endNum = position==0? 1: (int)vecList[position-1].size();

	//int j = 0;
	for (int j=0; j<endNum; j++)
	{	
		vecOutcome.clear();
		vecContext.clear();
		for (int i=0; i<TEMPLATE_NUM; i++)
		{
		combineOneFeature(position, j, i, feature);
		vecContext.push_back(feature);			
		feature.erase();
		}

		//temp << endl; //add
		Cmodel->MEmodel.eval_all(vecContext, vecOutcome);
		/*srcProb << "position: " << position << '\t' << "词:" << vec2paSen[position].first
			<< "prob:" << j << endl;*/
		//showvec2paContent2(vecOutcome, srcProb);
		seachBestPath(position, j, vecOutcome); 
	}
}

/*
 * Use BIESO tags or BIE tags tag. For example, 哈尔滨/ns#S-ns
 */
void IRNErecog::NEtaggingNormal(string& NEResult)
{
	for (int i=0,j=(int)vecNEResult.size()-1; i<(int)vec2paSen.size(); ++i, --j)
	{
		NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + "#" + vecNEResult[j] + " ";
	}
}

/*
 * Use []ne tag NEresult, using model based on BIO result
 */

void IRNErecog::getNEResult(string& result, int tagform)
{
	switch (tagform)
	{
	case FormBracket: NEtaggingBasedOnBIOForm1(result);
		break;
	case FormBIESO: NEtaggingBasedOnBIOForm2(result);
		break;
	case FormMerge: NEtaggingBasedOnBIOForm3(result);
		break;
	default: NEtaggingBasedOnBIOForm2(result);
		break;
	}
}

void IRNErecog::NEtaggingBasedOnBIOForm1(string& NEResult)
{
	bool flag = false;
	int i;
	int j;
	for (i=0, j=(int)vecNEResult.size()-1; i<(int)vec2paSen.size(); ++i, --j)
	{
		if (!isNEtype(vecNEResult[j], bIsNEtypeFlag))
		{
			if (flag == true)
			{ 
				NEResult.erase(NEResult.size()-1);
				NEResult += "]" + vecNEResult[j+1].substr(2)+ " ";    
				flag = false;																				
			}
			NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
			//NEResult += vec2paSen[i].first + " ";
			continue;
		}
		string TEMP = vecNEResult[j];
		switch (vecNEResult[j].at(0))
		{
			case 'B': if (flag == true)
					{ 
						NEResult.erase(NEResult.size()-1);
						NEResult += "]" + vecNEResult[j+1].substr(2)+ " ";
						flag = false;
					}
					NEResult += "[" + vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
					//NEResult += "[" + vec2paSen[i].first + " ";
					flag = true;
					break;
			case 'I': NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
					//NEResult += vec2paSen[i].first + " ";
					flag = true;
					break;
			case 'O': if (flag == true)
					{ 
						NEResult.erase(NEResult.size()-1);
						NEResult += "]" + vecNEResult[j+1].substr(2)+ " ";    
						flag = false;																				
					}
					NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
					//NEResult += vec2paSen[i].first + " ";
					break;
			default: break;
		}
	}
	if (flag)
	{
		NEResult.erase(NEResult.size()-1);
		NEResult += "]" + vecNEResult[j+1].substr(2)+ " ";
	}
}

void IRNErecog::NEtaggingBasedOnBIOForm2(string& NEResult)
{
	bool flag = false;
	int i;
	int j;
	for (i=0, j=(int)vecNEResult.size()-1; i<(int)vec2paSen.size(); ++i, --j)
	{
		if (!isNEtype(vecNEResult[j], bIsNEtypeFlag))
		{
			if (flag == true)
			{ 
				NEResult.erase(NEResult.size()-5);
				if (vecNEResult[j+1].at(0) == 'I')
				{
					NEResult += "E" + vecNEResult[j+1].substr(1) + " ";
				}
				else if (vecNEResult[j+1].at(0) == 'B')
				{
					NEResult += "S" + vecNEResult[j+1].substr(1) + " ";
				}
				flag = false;																				
			}
			NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
			//NEResult += vec2paSen[i].first + " ";
			continue;
		}
		string TEMP = vecNEResult[j];
		switch (vecNEResult[j].at(0))
		{
		case 'B': if (flag == true)
				  { 
					  NEResult.erase(NEResult.size()-5);
					  if (vecNEResult[j+1].at(0) == 'I')
					  {
						  NEResult += "E" + vecNEResult[j+1].substr(1) + " ";
					  }
					  else if (vecNEResult[j+1].at(0) == 'B')
					  {
						  NEResult += "S" + vecNEResult[j+1].substr(1) + " ";
					  }
					  flag = false;
				  }
				  NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + "#" + vecNEResult[j] + " ";
				  //NEResult += "[" + vec2paSen[i].first + " ";
				  flag = true;
				  break;
		case 'I': NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + "#" + vecNEResult[j] + " ";;
					//NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
					//NEResult += vec2paSen[i].first + " ";
				  flag = true;
				  break;
		case 'O': if (flag == true)
				  { 
					  NEResult.erase(NEResult.size()-5);
					  if (vecNEResult[j+1].at(0) == 'I')
					  {
						  NEResult += "E" + vecNEResult[j+1].substr(1) + " ";
					  }
					  else if (vecNEResult[j+1].at(0) == 'B')
					  {
						  NEResult += "S" + vecNEResult[j+1].substr(1) + " ";
					  }
					  flag = false;																				
				  }
				  NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + "#" + vecNEResult[j] + " ";
				  //NEResult += vec2paSen[i].first + " ";
				  break;
		default: break;
		}
	}
	if (flag)
	{
		NEResult.erase(NEResult.size()-5);
		if (vecNEResult[j+1].at(0) == 'I')
		{
			NEResult += "E" + vecNEResult[j+1].substr(1) + " ";
		}
		else if (vecNEResult[j+1].at(0) == 'B')
		{
			NEResult += "S" + vecNEResult[j+1].substr(1) + " ";
		}
	}
}


void IRNErecog::NEtaggingBasedOnBIOForm3(string& NEResult)
{
	bool flag = false;
	string strNE;
	for (int i=0, j=(int)vecNEResult.size()-1; i<(int)vec2paSen.size(); ++i, --j)
	{
		if (!isNEtype(vecNEResult[j], bIsNEtypeFlag))
		{
			NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
			continue;
		}
		string TEMP = vecNEResult[j];
		switch (vecNEResult[j].at(0))
		{
		case 'B': strNE = "";
				  getNEstring(i, j, strNE);
				  NEResult += strNE + " ";
				  break;
		case 'O': NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + "#O ";
				  break;
		default: break;
		}
	}
}

//下面是以[]NE的形式标记,下面针对，有BIESO的情况
void IRNErecog::NEtaggingBasedOnBIESO(string& NEResult)
{
	for (int i=0, j=(int)vecNEResult.size()-1; i<(int)vec2paSen.size(); ++i, --j)
	{
		switch (vecNEResult[j].at(0))
		{
		case 'B': NEResult += "[" + vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
			break;
		case 'S': NEResult += "[" + vec2paSen[i].first + "/" + vec2paSen[i].second +  "]" + vecNEResult[j].substr(2) + " ";
			break;
		case 'I':
		case 'O': NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + " ";
			break;
		case 'E': NEResult += vec2paSen[i].first + "/" + vec2paSen[i].second + "]" + vecNEResult[j].substr(2) + " ";
			break;
		default: break;
		}
	}
}

/*
 *	function: 
 */
void IRNErecog::combineOneFeature(int NENODEpos, int preNode, int FeatureNum, string& FeatureOut)
{
	int indexVector = 0;
	char chIndex[3];	
	string strTemp;

	for (int i=0; i<(int)Cmodel->vecTemplate[FeatureNum].size(); i++)
	{
		indexVector = NENODEpos + Cmodel->vecTemplate[FeatureNum][i].second;
		if (indexVector>=0 && indexVector<(int)vec2paSen.size())
		{
			if (i > 0)
			{
				FeatureOut += "/"; 
			}

			if (getFeature(indexVector, preNode, Cmodel->vecTemplate[FeatureNum][i].first, strTemp))  //特征中的一项
			{
				FeatureOut += strTemp;
			}
			else
			{
				FeatureOut.erase();
				break;
			}
		}
		else
		{
			FeatureOut.erase();
			break;
		}
	}
	
	if (FeatureOut.empty())
	{
		FeatureOut = "NULL";
	}
	// itoa(FeatureNum+1, chIndex, 10);
	sprintf(chIndex, "%d", FeatureNum + 1);
	strTemp = chIndex;
	FeatureOut = strTemp + "_" + FeatureOut;
}


/*
 *	It is used to get Features according to the specified index and information.
 *  If there is no word or pos to get, it will return false
 */

bool IRNErecog::getFeature(const int vecIndex, const int listIndex,
								const char FeatureChar, string& FeatureOut)
{
	string strTemp;
	switch (FeatureChar)
	{
	case 'W': FeatureOut = vec2paSen[vecIndex].first;
		return true;
	case 'P': FeatureOut = vec2paSen[vecIndex].second;
		return true;
	case 'T': FeatureOut = vecList[vecIndex][listIndex].NEtype;
		return true;
	case 'N': strTemp = vec2paSen[vecIndex].first;
		if (!getNEtagforHC(strTemp).empty())
		{
			FeatureOut = getNEtagforHC(strTemp);
			return true;
		}
		else return false;

	case 'S': if (vecIndex == 0)
			  {
				  FeatureOut = "S";
				  return true;
			  }
			  else return false;
	case 'E': if (vecIndex == vec2paSen.size()-1)
			  {
				  FeatureOut = "E";
				  return true;
			  }
			  else return false;
	case 'i': if (map_Niend[vec2paSen[vecIndex].first] == 1)
			  {
				  FeatureOut = "Niend";
				  return true;
			  }
			  else return false;
	case 's': if (map_Nsend[vec2paSen[vecIndex].first] == 2)
			  {
				  FeatureOut = "Nsend";
				  return true;
			  }
			  else return false;
	case 'z': if (map_Nzend[vec2paSen[vecIndex].first] == 3)
			  {
				  FeatureOut = "Nzend";
				  return true;
			  }
			  else return false;
	case 'H': if (map_preNh[vec2paSen[vecIndex].first] == 4)//H-1 preNh
			  {
				  FeatureOut = "PreNh";
				  return true;
			  }
			  else return false;
	case 'h': if (map_sufNh[vec2paSen[vecIndex].first] == 5) //h1 sufNh
			  {
				  FeatureOut = "SufNh";
				  return true;
			  }
			  else return false;

	default: return false;
	}
}
/*
 *	function: find the best path for the NE recognize
 */
void IRNErecog::seachBestPath(int posVec, int preNode, const vector< pair<string, double> >& vecProb)
{
	if (posVec == 0) 
	{
		dealFirstPathNode(posVec, vecProb);
	}
	else
	{
		dealOtherPathNode(posVec, preNode, vecProb);
	}
}

/*
 *	function: According to all the paths we have built, we find the best path with the hightest probability
 *      as the result path, and its NEtag is the best NEtype for the sentence
 */
void IRNErecog::getBestNEResult()
{
	string NEtype;
	int index = 0;

	vecNEResult.clear();
	for (int i=(int)vecList.size()-1; i>=0; i--)
	{
		if (i == vecList.size()-1)
		{
			index = getNodeIndexWithHighProb(i);
		}
		NEtype = vecList[i][index].NEtype;
		index = vecList[i][index].preNode;
		vecNEResult.push_back(NEtype);
		NEtype.clear();
	}
}

/*
 *	fuction: If it is the first node in the path, we only copy vecProb to the search path vector.
 */

void IRNErecog::dealFirstPathNode(int posVec, const vector< pair<string, double> >& vecProb)
{
	SEARCHNODE searchNode;
	int NodeNum = (int)vecProb.size()>SEARCHNODE_NUM? SEARCHNODE_NUM : (int)vecProb.size();
   
	for (int i=0; i<NodeNum; i++)
	{
		searchNode.NEtype = vecProb[i].first;
		searchNode.preNode = -1;
		searchNode.prob = -log(vecProb[i].second);	
		vecList[posVec].push_back(searchNode);
	}
}


/*
 *	fuction:   if it is not the first node, we should calculate the probability and some other work.
 *  parameter: int posVec is the vector ID of sentence information, int preNode is the 
 *          element ID of vecList[posVec] which is used to be T-1. vecProb is the result of fuction
 *          eval_all(). vecOut is the return result.
 */
void IRNErecog::dealOtherPathNode(int posVec, int preNode,
		        const vector< pair<string, double> >& vecProb)
{
	SEARCHNODE Node;
	double probability = 0;
	int i;
	int smallNodeIndex = -1;
	int sameNodeIndex = -1;
	int NodeNum = (int)vecProb.size()>SEARCHNODE_NUM? SEARCHNODE_NUM : (int)vecProb.size();
    
	if (vecList[posVec].size() == 0) 
	{
		//tempProb << "posVec:" << posVec << endl; //add
		
		for (i=0; i<NodeNum; i++)
		{
			Node.NEtype = vecProb[i].first;
			Node.preNode = preNode;
			Node.prob = -log(vecProb[i].second) + vecList[posVec-1][preNode].prob;
			vecList[posVec].push_back(Node);

			//tempProb << Node.NEtype << '\t'  //add
			//	     << Node.preNode << '\t'
			//		 << Node.prob << endl;
		}
	}
	else
	{
		//tempProb << "posVec:" << posVec << endl;

		for (i=0; i<NodeNum; i++)
		{
			probability = -log(vecProb[i].second) + vecList[posVec-1][preNode].prob;
			sameNodeIndex = getListNodeIndexWithSameType(vecProb[i].first, posVec);
			
      //      tempProb << vecProb[i].first << '\t'  //add
				  //   << preNode << '\t'
					 //<< probability << endl;

			if (sameNodeIndex == -1) 
			{
				smallNodeIndex = getNodeIndexWithSmallProb(posVec);
				if (smallNodeIndex != -1)
				{
					vecList[posVec][smallNodeIndex].NEtype = vecProb[i].first;
					vecList[posVec][smallNodeIndex].preNode = preNode;
					vecList[posVec][smallNodeIndex].prob = probability;				
				}
			}
			else
			{
				if (probability < vecList[posVec][sameNodeIndex].prob)
				{
					vecList[posVec][sameNodeIndex].NEtype = vecProb[i].first;
					vecList[posVec][sameNodeIndex].preNode = preNode;
					vecList[posVec][sameNodeIndex].prob = probability;					
				}
			}
		}
	}
}
/*
 *	function: judge the whether the searching node is empty
 */
bool IRNErecog::isSearchNodeEmpty(int posVec, int Listsize)
{
	if (posVec > Listsize)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 *	function: It will find the list node number which has the same NEtype with the input
 *     NEtype, and then return the number. If we can not find anyone fitting our request,
 *     we'll return -1 instead.
 *  parameter: NEtype is what we want to find in the list. posVec tells us the vecList index
 *  to find.   
 */
int IRNErecog::getListNodeIndexWithSameType(const string& NEtype, int posVec)
{
	for (int i=0; i<(int)vecList[posVec].size(); i++)
	{
		if (vecList[posVec][i].NEtype == NEtype)
		{
			return i;
		}
	}
	return -1;
}

/*
 *	function: Find the node index with the smallest probability. Because we have did -log()
 *      with the real probability, so the smallest node with the biggest -log(probability).
 */
int IRNErecog::getNodeIndexWithSmallProb(int posVec)
{
	double min = 0;
	int Index = -1;
	for (int i=0; i<(int)vecList[posVec].size(); i++)
	{
		if (vecList[posVec][i].prob >= min)
		{
			min = vecList[posVec][i].prob;
			Index = i;
		}		
	}

	return Index;
}

int IRNErecog::getNodeIndexWithHighProb(int posVec)
{
	double max = 1000;
	int Index = -1;
	for(int i=0; i<(int)vecList[posVec].size(); i++)
	{
		if (vecList[posVec][i].prob <= max)
		{
			max = vecList[posVec][i].prob;
			Index = i;
		}
	}
	return Index;
}

void IRNErecog::outvecList(vector<SEARCHVECTOR>& vecOut, ofstream& outfile)
{
	for (int i=0; i<(int)vecOut.size(); i++)
	{
		outfile << i << ":" << endl;
		for (int j=0; j<(int)vecOut[i].size(); j++)
		{
			outfile << vecOut[i][j].NEtype << "\t"
				    << vecOut[i][j].preNode << "\t"
					<< vecOut[i][j].prob << endl;
		}
	}
}


inline string IRNErecog::getNEtagforHC(const string& Word)
{
	if (Word == "某※※人")
		return "NH";
	else if (Word == "某※※地")
		return "NS";
	else if (Word == "某※※专")
		return "NZ";
	else if (Word == "某※※时")
		return "NT";
	else if (Word == "某※※日")
		return "NR";
	else if (Word == "某※※数")
		return "NM";
	else return "";

}

void IRNErecog::cutSingleNE(vector<string>& vecNE)
{
	for (int i=(int)vecNE.size()-1; i>0; --i)
	{
		string temp = vecNE[i];
		if ((vecNE[i]=="B-ni" || vecNE[i]=="B-ns" || vecNE[i]=="B-nz") && (i >= 1) && (vecNE[i-1].at(0) != 'I'))

		{
			vecNE[i] = "O";
		}
		else if (vecNE[i]=="B-nh" || vecNE[i]=="I-nh" || vecNE[i]=="B-nm" || vecNE[i]=="I-nm"
			|| vecNE[i]=="B-nr" || vecNE[i]=="I-nr" || vecNE[i]=="B-nt" || vecNE[i]=="I-nt")
		{
			vecNE[i] = "O";
		}

	}
}












void IRNErecog::getNEstring(int& senBegpos, int& NEBegpos, string& strOut)
{
	int senBeg = senBegpos;
	int NEBeg = NEBegpos;

	strOut += vec2paSen[senBeg].first;
	senBeg++;
	NEBeg--;
	while (NEBeg >= 0)
	{
		if (vecNEResult[NEBeg].at(0) == 'I')
		{
			strOut += vec2paSen[senBeg].first;
		}
		else
		{
			senBeg--;
			NEBeg++;
			break;
		}
		++senBeg;
		--NEBeg;
	}
	if (NEBeg < 0)
	{
		senBeg--;
		NEBeg++;
	}

	if (senBegpos == senBeg)
	{
		strOut += "/" + vec2paSen[senBeg].second + "#" + vecNEResult[NEBeg].substr(2);
	}
	else
		strOut += "/" + getNEPOS(vecNEResult[NEBeg]) + "#" + vecNEResult[NEBeg].substr(2);

	senBegpos = senBeg;
	NEBegpos = NEBeg;
}
string IRNErecog::getNEPOS(string& NEtype)
{
	string NEtag = NEtype.substr(2);
	if (NEtag == "ni")
	{
		return "ni";
	}
	else if (NEtag == "ns")
	{
		return "ns";
	}
	else if (NEtag == "nh")
	{
		return "nh";
	}
	else if (NEtag == "nz")
	{
		return "nz";
	}
	else if (NEtag == "nr")
	{
		return "nt";
	}
	else if (NEtag == "nt")
	{
		return "nt";
	}
	else if (NEtag == "nm")
	{
		return "m";
	}
	else
	{
		return "ni";
	}
}
