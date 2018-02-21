

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

typedef std::map<std::string,std::vector<int>> penguin;

penguin generateSumScoreListFromScores(penguin scorelist);
penguin generateScoreListFromScores(penguin scorelist);
penguin getNthRankings(penguin scorelist,int n);
std::string scorelistAsString(penguin scorelist);
std::vector<std::string> splitAt(std::string input,char splitter);
int getScoreAtTime(std::string civname,int partnum,penguin scorelist);
int weight(int inval);
bool isDead(int inval);
int parts;
penguin globscores;
bool natIsDead(std::string natname,int turnnum);
std::string getRedditTableFormatString(penguin rankings);
penguin getSidewaysRankings(penguin scorelist,int maxnum,int atturn);

int main(int argc, const char * argv[]) {
	// insert code here...
	std::ifstream file = std::ifstream("/absolute/path/to/file/exported/from/spreadsheets/of/rankings.csv");
	std::string val;
	std::vector<std::vector<std::string>> loadeds = {};
	//grab csv stuffs
	while (file.good()) {
		getline(file,val);
		auto splitted = splitAt(val, ',');
		splitted.erase(splitted.begin());//first element is gonna be "Part,0,1,2,3,4..." - not what we want!
		loadeds.push_back(splitted);
	}
	//loadeds is now something like {{"Boers","Hawaii",...},{1,60,...},{1,59,...},...}
	//and we want it to be like {"Boers":{1,1,1,1,1,...},"Hawaii":{60,59,58,57,...},...}
	penguin scores = {};
	long sz = loadeds[0].size();
	for (int i = 0;i<sz;i++) {
		std::string natname = loadeds[0][i];
		std::vector<int> rankings = std::vector<int>(loadeds.size()-2);
		for (int j = 1;j<loadeds.size()-1;j++) {
			try {rankings[j-1] = std::stoi(loadeds[j][i]);}
			catch (std::invalid_argument e) {rankings[j-1] = -1;}//for if they left it blank for 'dead'
		}
		scores[natname]=rankings;
		parts = (int)rankings.size();
	}
	globscores = scores;
	//now the prep is done!
	//vvv Uncomment if you want all the values everywhere vvvv
	//std::cout << scorelistAsString(getNthRankings(scores,3)) << "\n";
	//Otherwise, this prints out the rankings for the current turn
	//std::cout << getRedditTableFormatString(getNthRankings(scores, 2)) << "\n";
	//And this gives the sideways-rankings
	//std::cout << scorelistAsString(getSidewaysRankings(scores, 15, 101));
	//And this gives rankings of sideways-rankings at the 'current turn'
	std::cout << getRedditTableFormatString(getNthRankings(getSidewaysRankings(scores, 15, 101),2));
	
}

penguin getSidewaysRankings(penguin scorelist,int maxnum,int atturn) {
	penguin toReturn = {};
	for (auto nation : scorelist) {
		toReturn[nation.first] = {};
	}
	for (int i = 0;i<maxnum;i++) {
		scorelist = generateScoreListFromScores(scorelist);
		for (auto nation : scorelist) {
			toReturn[nation.first].push_back(nation.second[atturn-1]);
		}
	}
	return toReturn;
}

std::string getRedditTableFormatString(penguin rankings) {
	std::string toReturn = "Position | Civ\n:------|:-------\n";
	std::vector<std::pair<std::string,int>> verticalSlice = {};
	for (auto nation : rankings) {
		//loop through every nation, add it to vertical slice
		verticalSlice.push_back({nation.first,nation.second[nation.second.size()-1]});
	}
	sort(verticalSlice.begin(), verticalSlice.end(),
		[=](std::pair<std::string, int>& a, std::pair<std::string, int>& b) {
			return a.second < b.second;
		}
	);
	for (auto nation : verticalSlice) {
		toReturn+=nation.first+" | "+std::to_string(nation.second)+"\n";
	}
	return toReturn;
}

int getScoreAtTime(std::string civname,int partnum,penguin scorelist) {
	int toReturn = 0;
	for (int i = partnum;i>=0;i--) {
		toReturn+=weight(scorelist[civname][i]);
	}
	return toReturn;
}

bool natIsDead(std::string natname,int turnnum) {
	return isDead(globscores[natname][turnnum]);
}

bool isDead(int inval) {return inval==-1;}

int weight(int inval) {
	if (isDead(inval)) {return 0;}
	return 61-inval;
	/*
	return ((const int[]){
		320,290,266,247,232,220,210,201,192,184,
		176,168,161,154,147,140,134,128,122,116,
		110,105,100, 95, 90, 85, 80, 76, 72, 68,
		 64, 60, 56, 52, 49, 46, 43, 40, 37, 34,
		 31, 28, 26, 24, 22, 20, 18, 16, 14, 12,
		 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
		 0
	})[inval-1];*/
}


penguin generateSumScoreListFromScores(penguin scorelist) {
	penguin toReturn = {};
	for (auto nation : scorelist) {
		std::vector<int> scoresAtTime = {};
		for (int i = 0;i<nation.second.size();i++) {
			scoresAtTime.push_back(getScoreAtTime(nation.first,i,scorelist));
		}
		toReturn.insert({nation.first,scoresAtTime});
	}
	return toReturn;
}

penguin generateScoreListFromScores(penguin scorelist) {
	penguin sumscores = generateSumScoreListFromScores(scorelist);
	penguin toReturn = sumscores;//clone this for now - we'll change its values in the loop
	for (int turn = 0;turn<scorelist["Boers"].size();turn++) {
		std::vector<std::pair<std::string,int>> verticalSlice = {};
		for (auto nation : sumscores) {
			//loop through every nation, add it to vertical slice
			verticalSlice.push_back({nation.first,nation.second[turn]});
		}
		sort(verticalSlice.begin(), verticalSlice.end(),
			[=](std::pair<std::string, int>& a, std::pair<std::string, int>& b) {
    			return a.second > b.second;
			}
		);
		//now find each nation and add its score to the return list
		for (int i = 0;i<verticalSlice.size();i++) {
			toReturn[verticalSlice[i].first][turn] = i+1;//add 1 because ranks start from 1 not 0
		}
	}
	return toReturn;
}

std::string scorelistAsString(penguin scorelist) {
	std::string toReturn = "";
	for (auto nation : scorelist) {
		toReturn+="----"+nation.first + "----\n";
		for (auto n : nation.second) {
			toReturn+=std::to_string(n)+",";
		}
		toReturn+="\n";
	}
	return toReturn;
}

penguin getNthRankings(penguin scorelist,int n) {
	if (n<=1) {
		return scorelist;
	}
	else if (n==2) {
		return generateScoreListFromScores(scorelist);
	}
	else {
		return getNthRankings(generateScoreListFromScores(scorelist), n-1);
	}
}

std::vector<std::string> splitAt(std::string input,char splitter) {
	//can we just take a minute to be angry about how C++ has no string split library function?
	
	
	
	
	//hey, stop reading - you haven't waited a minute yet!
    std::vector<std::string> toReturn = {""};
    int index = 0;
    for(int n = 0;n<input.size();n++) {
        if (input[n]==splitter) {
            toReturn.push_back("");
            index++;
        }
        else {
            toReturn[index]+=input[n];
        }
    }
    return toReturn;
}
