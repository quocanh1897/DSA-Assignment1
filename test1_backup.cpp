#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include "tinyxml.h"
#include "BKavltree.h"
#include <ctime>
#include<fstream>
#include<sstream>
#include<stack>
#include<algorithm>
#include<thread>
#include <boost/date_time.hpp>

using namespace boost::filesystem;
struct Checker {
	bool* startRun = new bool(false);
	string* nameExe = new string("");
	bool* signal = new bool(false);
};
//------------->>>>  PROTOTYPE  <<<<-------------//

bool DeleteSubFolder(avlTree &Data, path workingDir, string ID, string sub);
bool CopyfileStoW(path workingDir, path submitFol, string ID, string sub);

bool checkID(avlTree* dataID, path submitFolder, path workingDir, Checker* checkErrorExe,Heap* Priority);
bool CreateXML(path submitfolder, string ID, string sub);

void exportScore(path workingDir, avlTree* dataIN, string ID);
int compileFile(path FolderWD, string ID, string sub);
void scoreOutput(path workingDir, string ID, string fileToScore, int subNumber, int numTestcase);
void runThenScoreFileSub(path workingDir, string ID, avlTree* dataID, int numOfSubIn,int count, Checker* checkErrorExe);
void scoreSub(path workingDir, string ID, int subNumber, avlTree* dataIn);
void AddPriority(Heap* Priority, string gettime, string ID);

//------------->>>>  IMPLEMENT <<<<-------------//

bool DeleteSubFolder(avlTree &Data, path workingDir, string ID, string sub) {
	if(Data.search(ID)) Data.search(ID)->numberSub--;
	path subfolder = workingDir / ID / sub;
	for (directory_iterator file(subfolder); file != directory_iterator(); ++file) {
		string filename = file->path().filename().string();//lay ra ten file
		path temp = subfolder / filename;
		remove(temp);
	}
	remove(workingDir / ID / sub);
	return true;
}

int compileFile(path FolderWD, string ID, string sub) {
	create_directory(FolderWD / ID / sub / "build");
	int count = 0;
	while (1) {
		count++;
		string makefile = "makefile" + to_string(count);
		if (exists(FolderWD / "testcase" /makefile)) {
			copy_file(FolderWD / "testcase" / makefile, FolderWD / ID / sub / makefile);
			string cmdstr = "pushd " + (FolderWD / ID / sub).string() + "&& make -f " + makefile + " buildcpp";
			system(cmdstr.c_str());
		}
		else break;
	}
	return count - 1;
}

bool CopyfileStoW(path workingDir, path submitFol,string ID,string sub) {
	bool flag = false;//da tim thay quantity chua
	int quantity = 0;//so luong file
	int count = 0;//dem
	path IDFolderInWorkingDir = workingDir /ID/sub;
	path IDFolderInSubmitFol = submitFol /ID/sub;
		//this is a brand new IDFolder
	create_directory(workingDir / ID);
	create_directory(workingDir / ID/sub);
	for (directory_iterator file(IDFolderInSubmitFol);file != directory_iterator(); ++file) {
		//dem file da duoc doc sang
		count++;
		string curFileName = file->path().filename().string();
		path desFileName = workingDir /ID/sub/curFileName;
		if (!exists(desFileName) ||
			exists(desFileName) &&
			file_size(desFileName) != file_size(file->path())) {
			copy(file->path(), workingDir / ID/sub/ curFileName);
		}


		//tim so luong file
		if (!curFileName.compare("pro.xml")) {
			//chuyen string ve char* de dung ham doc
			char * writable = new char[desFileName.string().size() + 1];
			for (int i = 0; i < desFileName.string().length()+1; i++) {
				if (i == desFileName.string().length()) 
					writable[i] = '\0';
				else 
					writable[i] = desFileName.string()[i];
				
			}
			TiXmlDocument doc(writable);
			if (!doc.LoadFile())
			{
				//printf("%s", doc.ErrorDesc());
				return 0;
			}
			TiXmlElement* root = doc.RootElement();
			TiXmlElement* child1 = root->FirstChildElement();
			while (child1) {
				string temp = "quantity";
				if (!temp.compare(child1->ValueTStr().c_str()))
				{
					quantity = atoi(child1->GetText());
					flag = true;
					if (quantity == count&&flag) return true;
					break;
				}
				child1 = child1->NextSiblingElement();
			}		
		}
		if (quantity == count&&flag) return true;
	}
	return false;
}

bool Replacefile(path IDFolder, string newsub) {
	bool flag = false;//da tim thay quantity chua
	int quantity = 0;//so luong file
	int count = 0;//dem
				  //xet xem co phai thu muc hoan toan la folder khong
	int isAllDir = true;
	for (directory_iterator file(IDFolder); file != directory_iterator(); ++file) {
		string curFileName = file->path().filename().string();
		path desFileName = newsub / curFileName;
		//dem file
		if (is_directory(file->path())) continue;
		else isAllDir = false;//neu k phai folder -> false;
		count++;
		if (exists(newsub / curFileName)) return true;
		copy_file(file->path(), newsub / curFileName);

		//tim so luong file
		if (!curFileName.compare("pro.xml")) {
			//chuyen string ve char* de dung ham doc
			char * writable = new char[desFileName.string().size() + 1];
			for (int i = 0; i < desFileName.string().length() + 1; i++) {
				if (i == desFileName.string().length())
					writable[i] = '\0';
				else
					writable[i] = desFileName.string()[i];

			}
			remove(file->path());
			TiXmlDocument doc(writable);
			if (!doc.LoadFile())
			{
				//printf("%s", doc.ErrorDesc());
				return 0;
			}
			TiXmlElement* root = doc.RootElement();
			TiXmlElement* child1 = root->FirstChildElement();
			while (child1) {
				string temp = "quantity";
				if (!temp.compare(child1->ValueTStr().c_str()))
				{
					quantity = atoi(child1->GetText());
					flag = true;
					if (quantity == count&&flag) return true;
					break;
				}
				child1 = child1->NextSiblingElement();
			}
		}
		remove(file->path());
		if (quantity == count&&flag) return true;
	}
	if (isAllDir) return true;
	return false;
}
void ReadXML(path IDFolder, string sub, Heap* Priority) {
	string gettime;
	string curFileName = "pro.xml";
	path desFileName = IDFolder / sub/curFileName;
	//chuyen string ve char* de dung ham doc
	char * writable = new char[desFileName.string().size() + 1];
	for (int i = 0; i < desFileName.string().length() + 1; i++) {
		if (i == desFileName.string().length())
			writable[i] = '\0';
		else
			writable[i] = desFileName.string()[i];

	}
	TiXmlDocument doc(writable);
	if (!doc.LoadFile())
		{
		//printf("%s", doc.ErrorDesc());
		return;
		}
	TiXmlElement* root = doc.RootElement();
	TiXmlElement* child1 = root->FirstChildElement();
	while (child1) {
		string time = "time";
		if (!time.compare(child1->ValueTStr().c_str()))
			{
				gettime = child1->GetText();
				AddPriority(Priority, gettime, IDFolder.filename().string());
			}
		child1 = child1->NextSiblingElement();
	}
}

bool CreateXML(path submitfolder, string ID, string sub) {

	//tao file xml
	path subfile = submitfolder / ID / sub;
	path exist = subfile / "pro.xml";
	if (exists(exist)) return 0;
	TiXmlDocument doc;
	TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "UTF-8", "");
	doc.LinkEndChild(dec);
	//root
	TiXmlElement* root = new TiXmlElement("Source");
	doc.LinkEndChild(root);

	int i = 0;
	//file

	for (directory_iterator file(subfile); file != directory_iterator(); ++file) {
		string namefile = file->path().filename().string();//lay ra ten ID
		string str = namefile.substr(namefile.find(".") + 1, namefile.length() - 1);
		const char* duoi = str.c_str();
		const char* file_name = namefile.c_str();
		TiXmlElement* child = new TiXmlElement(duoi);
		TiXmlText *file_name_text = new TiXmlText(file_name);
		root->LinkEndChild(child);
		child->LinkEndChild(file_name_text);
		i++;
	}
	{
		TiXmlElement* child = new TiXmlElement("xml");
		TiXmlText *file_name_text = new TiXmlText("pro.xml");
		root->LinkEndChild(child);
		child->LinkEndChild(file_name_text);
	}
	{
		TiXmlElement* quantity = new TiXmlElement("quantity");
		root->LinkEndChild(quantity);
		TiXmlText *quantity_num = new TiXmlText(to_string(i + 1).c_str());
		quantity->LinkEndChild(quantity_num);
	}
	path xmlfile = subfile / "pro.xml";
	string link = xmlfile.string();
	const char* s1 = link.c_str();
	//char* temp = new char(xmlfile.string().length() + 1);
	//memcpy(temp, s0, xmlfile.string().length() + 1);
	doc.SaveFile(s1);

	return 1;
}

bool checkID(avlTree* dataID, path submitFolder, path workingDir,Checker* checkErrorExe,Heap* Priority) {

	//duyet tuan tu file submitFolder
		if (Priority->isEmpty()) return 0;
		nodeHeap temp;
		Priority->heapDelete(temp);
		string IDNameFolder = temp.ID;//lay ra ten ID														//ID bo vao database neu chua co
		node* IDNode = dataID->search(IDNameFolder);	
		int count = 1;
		if (!IDNode)
			return 0;
		//duyet tuan tu file sub
		path IDfolder = workingDir / IDNameFolder;
		if (!exists(IDfolder)) {}
		else {
			for (directory_iterator file(IDfolder); file != directory_iterator(); ++file) 
				count++;
		}
				if (IDNode->isLoading) return 0;//neu fileID dang load thi di ra ngoai
				string ID = IDNameFolder;
				int numOfSub = count;
				string sub = "sub" + to_string(count);
				//copy den khi nao du file thi thoi nho` vao quantity trong xml
				while (1) {
					if (CopyfileStoW(workingDir, submitFolder, ID, sub) == true) break;
				}
				if (exists(workingDir / ID / sub / "build")) return 0;
				int countTest=compileFile(workingDir, ID, sub);
				//--------------------------------//
				runThenScoreFileSub(workingDir, ID, dataID, numOfSub, countTest, checkErrorExe);
				return 1;
}

void reFormatTxt(string pathFile) {

	std::ifstream fin(pathFile);
	std::ofstream fout("temp.txt");
	string s;
	while (getline(fin, s)) { // read a line at a time
		stringstream line(s);
		while (line >> s) { // read the words, eliminate all extra blank space
			fout << s << " ";
		}
		fout << endl; // put the newline back
	}
	fin.close();
	fout.close();

	std::ifstream ifs("temp.txt");
	string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();
	//str.erase(std::remove(str.begin(), str.end(), '\n \n'), str.end());
	std::string::size_type i = 0;
	while (i < str.length()) {
		i = str.find("\n\n", i);
		if (str[str.length() - 1] == '\n')
			str.erase(str.length() - 1, 1);
		if (i == std::string::npos) {
			break;
		}
		str.erase(i, 1);
	}
	std::ofstream ofs(pathFile);
	ofs << str;
	ofs.close();

}

void scoreOutput(path workingDir, string ID, string fileToScore, int subNumber, int numTestcase) {
	int dem = 0;
	int a[50];
	string fileChange = fileToScore.substr(0, fileToScore.find("."));
	//node *numofSub = dataID->search(ID);
	string s = "sub" + to_string(subNumber);
	path pathDaFile = workingDir / ID / s / "build" ;
	string daFile = pathDaFile.string() + "\\outputda" + fileChange + "_" + to_string(numTestcase) + ".txt";
	
	//xoa dong trong o testcase
	reFormatTxt(daFile);
	
	// so luong phan tu cua dap an
	std::ifstream in1;
	//in1.open(daFile);
	in1.open(daFile);
	while (!in1.eof())
	{
		string temp1 = "";
		getline(in1, temp1);
		dem = dem + 1;
	}
	in1.close();
	/////////////////////////////
	std::ifstream in1_1;
	in1_1.open(daFile);//doc DA giam khao
	std::ifstream in2;
	string thisinhFile = pathDaFile.string() + "\\output" + fileChange + "_" + to_string(numTestcase) + ".txt";
	
	reFormatTxt(thisinhFile);

	in2.open(thisinhFile);//doc DA thi sinh
	int dung = 0;
	for (int k = 0; k <= dem - 2; k++)
	{
		string temp1 = "";
		getline(in1_1, temp1);
		string temp2 = "";
		getline(in2, temp2);
		if (temp1 == temp2)//so DA
		{
			dung = dung + 1;// dem so luong kq dung
		}
	}
	in1_1.close();
	in2.close();
	double diem = (dung) / (double)(dem - 1);//tan so dung
	std::fstream out;
	string scoreFile = pathDaFile.string() + "\\scoreOf" + fileChange + ".txt";

	out.open(scoreFile, ios_base::app);
	if (numTestcase == 5) {
		out << "testcase " << numTestcase << ": " << diem * 10 << endl;
		out.close();
		
		//open file to calculate total score
		out.open(scoreFile, ios_base::in);
		float *score = new float[5];
		for (int i = 0; i < 5; i++) {
			string line = "";
			int num;
			getline(out, line);
			
			stringstream tempLine(line);
			tempLine >> line >> num >> line >> score[i];

		}
		out.close();
		out.open(scoreFile, ios_base::app);
		float sum = 0;
		for (int i = 0; i < 5; i++) {
			sum += score[i];
		}
		out << "Total: " << sum / 5;
		out.close();
		return;
	}

	out << "testcase " << numTestcase << ": " << diem * 10 << endl;// luu tan so dung vao file output
	out.close();
	return;
}

void exportScore(path workingDir, avlTree* dataIN, string ID) {
	if (dataIN->root == NULL)
		return;

	path listSV = workingDir / "listSV.csv";
	path listAllSubSV = workingDir / "listAllSubSV.csv";
	string pathListSV = listSV.string();
	string pathListAllSubSV = listAllSubSV.string(); 
	std::ofstream myfile;
	std::ofstream myfileAS;
	
	//<-----print to listAllSubSV.csv all of subs ----->//
	if (!exists(listAllSubSV)) {
		myfileAS.open(pathListAllSubSV);
		myfileAS << "MSSV, Lan nop, Diem\n";
		myfileAS.close();
	}
	myfileAS.open(pathListAllSubSV, ios_base::app);
	node *sv = dataIN->search(ID);

	string cmdTempAS = sv->key + "," + to_string(sv->numberSub) + "," + to_string(sv->scoreStack.top()) + "\n";
	myfileAS << cmdTempAS;

	//<-----print to listSV.csv max score of subs----->//
	myfile.open(pathListSV);
	myfile << "MSSV, So lan nop, Diem cao nhat\n";
	//traverse avlTree data to find all MSSV
	node *current = dataIN->root;
 
	stack<string> tempstack;
	bool done = 0;

	while (!done)
	{
		if (current != NULL){
			tempstack.push(current->key);
			current = current->left;
		}
		else{
			if (!tempstack.empty())	{
 
				node *t1 = dataIN->search(tempstack.top());
 
				current = t1;//node chua MSSV, sub, score
				tempstack.pop();

				//process here

				string cmdTemp = current->key + "," + to_string(current->numberSub) + "," + to_string(current->scoreHeap->getMax()) + "\n" ;
				myfile << cmdTemp;


				/* we have visited the node and its left subtree.
				Now, it's right subtree's turn */
				current = current->right;
			}
			else
				done = 1;
		}
	} /* end of while */
	myfile.close();
	myfileAS.close();

	std::ofstream outTree("avl.dat");
	dataIN->saveAVL(dataIN->root, outTree);
	outTree.close();

}
 
void scoreSub(path workingDir, string ID, int subNumber, avlTree* dataIn) {
 
	int dem = 0;
	string s = "sub" + to_string(subNumber);
	path pathDaFile = workingDir / ID / s / "build";
	string scoreFile1 = pathDaFile.string() + "\\scoreOf1.txt";
	string scoreFile2 = pathDaFile.string() + "\\scoreOf2.txt";
	string totalScore = pathDaFile.string() + "\\score.txt";

	std::fstream in;
	string line;
	float score1 = 0, score2 = 0;
	//open scoreOf1 to save score of program1
	in.open(scoreFile1, ios_base::in);
	for (int i = 0; i < 5; i++) {
		string temp;
		getline(in, temp);
	}
	getline(in, line);
	stringstream tempLine(line);
	tempLine >> line >> score1;
	in.close();

	//open scoreOf2 to save score of program2
	in.open(scoreFile2, ios_base::in);
	for (int i = 0; i < 5; i++) {
		string temp;
		getline(in, temp);
	}
	getline(in, line);
	stringstream tempLine2(line);
	tempLine2 >> line >> score2;
	in.close();

	//doc setting thang diem o file config
	std::ifstream ifs("settings.config");
	string l;
	getline(ifs, l); getline(ifs, l); getline(ifs, l); getline(ifs, l); getline(ifs, l);
	int star1 = l.find("\"");
	int star2 = l.find("%");
	string ll = l.substr(l.find("+"), l.length() - 1);
	int star3 = ll.find("\"");
	int star4 = ll.find("%");
	string t1 = l.substr(star1 + 1, star2-star1-1);
	string t2 = ll.substr(star3 + 1, star4-star3-1);
	stringstream ss(t1);
	float a = 100, b = 0;
	ss >> a;
	stringstream ss2(t2);
	ss2 >> b;

	std::fstream out;
	out.open(totalScore, ios_base::app);
	out << score1*a/100 + score2*b/100;
	out.close();

	//luu diem vao heap & stack cua rieng tung sinh vien
 
	node *SV = dataIn->search(ID);
 
	SV->scoreStack.push(score1*0.3 + score2*0.7);
	SV->scoreHeap->heapInsert(nodeHeap(score1*0.3 + score2*0.7));

	exportScore(workingDir, dataIn, ID);
	return;
}

void runThenScoreFileSub(path workingDir, string ID, avlTree* dataID, int numOfSubIn,int count,Checker* checkErrorExe) {
	node *numofSub = dataID->search(ID);
 
	if (numofSub == NULL) {
		return;
	}

	//run numofSub times this function
	for (int i = 0; i < numofSub->numberSub; i++) {
		string s = "sub" + to_string(i + 1);
		path build = workingDir / ID / s / "build";

		path scoreFile = build / "score.txt";
		if (!exists(build) || exists(scoreFile)) {
			continue;
		}

		//run code per objFile
		for (int j = 1; j <= count;j++) {
			string cdDirectory = "pushd " + (workingDir / ID / s).string();
			string cmdCreateExe= cdDirectory + " && make -f makefile" + to_string(j) + " createexe";
			system(cmdCreateExe.c_str());
			for (int numTestcase = 1; numTestcase <= 5; numTestcase++) {
				//copy testcase\input1.txt -> build\input.txt
				string fileChange = to_string(j);
				string inputNum = "testcase\\input" + fileChange + '_' + to_string(numTestcase) + ".txt";
				path inputFile = workingDir / inputNum;
				string tempname = "input" + fileChange + ".txt";
				path reNameInputFile = build / tempname;
				copy_file(inputFile, reNameInputFile);

				
				string cmdRunFileToScore = cdDirectory + " && cd build && " + fileChange + ".exe";
				//tin hie.u startRun =true
				*(checkErrorExe->nameExe) = fileChange; *(checkErrorExe->startRun) = true;
				system(cmdRunFileToScore.c_str());
				//neu on thi reset startRun
				*(checkErrorExe->startRun) = false;
				//signal tra ve true khi file exe co van de
				if (*(checkErrorExe->signal)) {
					*(checkErrorExe->signal) = false;
					break;
				}
				path objfile{ workingDir / ID / s / "build" / (fileChange + ".obj").c_str() };
				if (!exists(objfile)) break;

				
				string outputNum = "output" + fileChange + '_' + to_string(numTestcase) + ".txt";
				string tempnameout = "output" + fileChange + ".txt";
				path outputFile = build / tempnameout;
				path reNameOutputFile = build / outputNum;
				rename(outputFile, reNameOutputFile);

				remove(reNameInputFile);
				string daNum = "outputda" + fileChange + '_' + to_string(numTestcase) + ".txt";
				path daFile = workingDir / "testcase" / daNum;
				path reNameDAFile = build / daNum;
				copy_file(daFile, reNameDAFile);
				scoreOutput(workingDir, ID, fileChange, i+1, numTestcase);
				remove(reNameDAFile);
			}
			//xoa obj exe
			string cmdClean = cdDirectory + " && make -f makefile" + to_string(j) + " clean";
			system(cmdClean.c_str());
		}


		scoreSub(workingDir, ID, i+1, dataID);
	}

}
 
void ThreadCompile(avlTree* DataID, path submitFolder, path workingDir, Checker* checkErrorExe, Heap* Priority) {
 
	while (1) {
		checkID(DataID, submitFolder, workingDir, checkErrorExe, Priority);
	}
}
void Traverse(path submitFolder, Heap* Priority, avlTree* DataID) {
	for (directory_iterator fileID(submitFolder); fileID != directory_iterator(); ++fileID) {
		int count = 0;
		node* IDNode = DataID->search(fileID->path().filename().string());
		if (!IDNode) continue;
		if (IDNode->isLoading) continue;
		for (directory_iterator file(fileID->path()); file != directory_iterator(); ++file) 
			if (is_directory(file->path())) {
				count++;
			}
			else continue;
		if (count > IDNode->numberSub) {
				IDNode->numberSub++;
				ReadXML(fileID->path(), "sub" + to_string(IDNode->numberSub), Priority);m
		}
	}
 }
void PrepareCompile(avlTree* DataID,path submitFolder,Heap* Priority) {
 
	for (directory_iterator fileID(submitFolder); fileID != directory_iterator(); ++fileID) {
		node* IDNode = DataID->search(fileID->path().filename().string());
		if (!IDNode)
			DataID->AVLInsert(DataID->root, IDNode = new node(fileID->path().filename().string(), 0), DataID->taller);
		//dem co bao nhieu sub
		int count = 0;
		//co su thay doi gi khong-> co can sinh file khong
		bool CanCreateSub = 0;
		for (directory_iterator file(fileID ->path()); file != directory_iterator(); ++file) {
			if (is_directory(file->path())) { 
				count++;
			}
			else CanCreateSub = 1;
		}

		//neu khong can tao folder thi continue
		if (!CanCreateSub) {
			continue;
		}
			IDNode->isLoading = true;
			//tao sub moi
			path newsub = fileID->path() / ("sub" + to_string(count + 1)).c_str();
			create_directory(newsub);
			while (1) {
				if (Replacefile(fileID->path(), newsub.string())) break;
			}

			IDNode->isLoading = false;
	}
}
 
void ThreadPrepareCompile(avlTree* DataID,path submitFolder,Heap* Priority) {
 
	while (1) {
		Traverse(submitFolder, Priority, DataID);
		PrepareCompile(DataID,submitFolder,Priority);
	}
}

void settingConfig(path &SF, path &WD, path &uploadedFolder) {
	std::ifstream ifs("settings.config");
	string line, sf, wd, uploaded;

	getline(ifs, line);
	sf = line.substr(line.find("\"") + 1, line.find("\"") );
	path t(sf);
	SF = t;

	getline(ifs, line);
	wd = line.substr(line.find("\"") + 1, line.length());
	wd.erase(wd.find("\""), 1);
	path t2(wd);
	WD = t2;

	getline(ifs, line);
	uploaded = line.substr(line.find("\"") + 1, line.length() - 1);
	uploaded.erase(uploaded.find("\""),1);
	path t3(uploaded);
	uploadedFolder = t3;
}
//checkerError
void checkErrorExe(Checker* checkErrorExe) {
	//signal dung de phat hien exe co van de, va se khong thuc thi tiep
	//bat dau dem thoi gian
	std::ifstream ifs("settings.config");
	string line;
	getline(ifs, line);	getline(ifs, line);	getline(ifs, line); getline(ifs, line);

	int star1 = line.find("\"");
	int star2 = star1 + line.substr(star1 + 1, line.length() - 1).find("\"");
	string t = line.substr(star1 + 1, star2);
	string temp = t.substr(0, t.find("\""));
	stringstream ss(temp);
	float time = 5;
	ss >> time;

	if (!*(checkErrorExe->startRun)) return;
	double startTime = clock();
	while (1) {
		if (!*(checkErrorExe->startRun)) break;
		double deltaTime = (clock() - startTime) / (double)CLOCKS_PER_SEC;
		//co hoi cuoi cung la x sec cho sv
		if (deltaTime > time) {
			*(checkErrorExe->signal) = true;
			string cmd = "pushd C:\\Windows\\System32 && Taskkill /F /IM " + *(checkErrorExe->nameExe) + ".exe";
			system(cmd.c_str());

			break; 
		}
	}
	*(checkErrorExe->nameExe) = "";
	*(checkErrorExe->startRun) = false;
}
void ThreadCheckErrorExe(Checker* ErrorExe) {
	while (1) {
		checkErrorExe(ErrorExe);
	}
}
void AddPriority(Heap* Priority, string gettime, string ID) {
	stringstream sstr;
	sstr << gettime;
	boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
	int month, day, hour, min, second;
	char a[4];
	sstr >> month >> a[0] >> day >> a[1] >> hour >> a[2] >> min >> a[3] >> second;
	//create key priority
	double timevalue = (timeLocal.date().month().as_enum() - month) * 44640 + (timeLocal.date().day() - day) * 1400 + (timeLocal.time_of_day().hours() - hour) * 60 + (timeLocal.time_of_day().minutes() - min) + (timeLocal.time_of_day().seconds() - second) / (double)60;
	int heso = atoi(ID.c_str())/100000;
	double key = timevalue*heso;
	Priority->heapInsert(nodeHeap(key, ID));
}
int main() {
	Checker* checkErrorExe = new Checker();
	path submitFolder, workingDir, uploadedFolder;
	settingConfig(submitFolder, workingDir, uploadedFolder);
	Heap* Priority = new Heap(1000);
	avlTree* DataID=new avlTree();
	DataID->root = NULL;
	std::ifstream inTree("avl.dat");
	DataID->loadAVL(DataID->root, inTree);
	inTree.close();
	thread t2(ThreadCompile, DataID, submitFolder, workingDir,checkErrorExe,Priority);
	thread t1(ThreadPrepareCompile, DataID, submitFolder,Priority);
	thread t3(ThreadCheckErrorExe,checkErrorExe);
	t1.join();
	t2.join();
	t3.join();
	//
	//std::cout << "Current System Time = " << (double) << std::endl;
	//system("pause");
}