#include <windows.h>
#include <stdio.h>
#include <bits/stdc++.h>
#include <string.h>
#include <cstdlib>
using namespace std;
struct bootSector {
	string fat_type;
	int byte_per_sec;
	int sec_per_clu;
	int sec_extra_bootSec;
	int fat_num;
	int total_sector;
	int sec_per_fat;
	int fat1_position;
	int rdet_position;
	int data_position;

};

struct entry_temp {
	string offset1_10;   //5 ki tu unicode
	string offsetE_12;   // 6 ki tu
	string offfset1C_4;  // 2 ki tu
};
struct entry_main {
	string offset0_8; //ten tap tin ngan
	string offset8_3; //phan mo rong
	int offset14_2;  //cluster bat dau( 2 byte cao)
	int offset1A_2;  //cluster bat dau(2 byte thap)
	int offset1C_4;
};
struct entry {
	entry_main main;
	vector<entry_temp> list_temp;
};
short ReadSect // Doc Sector thu _nsect vao mang DATA
(const char *_dsk,    // Name of disk
 BYTE DATA[512]  ,    // DATA store BYTE in one sector
 unsigned int _nsect   // Sector number
) {
	memset(DATA, 0, 512);
	DWORD dwBytesRead(0);

	HANDLE hFloppy = NULL;
	hFloppy = CreateFile(_dsk,    // Floppy drive to open
	                     GENERIC_READ,              // Access mode
	                     FILE_SHARE_VALID_FLAGS,           // Share Mode
	                     NULL,                      // Security Descriptor
	                     OPEN_EXISTING,             // How to create
	                     0,                         // File attributes
	                     NULL);                     // Handle to template

	if(hFloppy != NULL) {
		SetFilePointer(hFloppy,_nsect*512,0,FILE_BEGIN);
		// Read the boot sector
		if (!ReadFile(hFloppy, DATA, 512, &dwBytesRead, NULL)) {
			printf("Error in reading floppy disk\n");
		}

		CloseHandle(hFloppy);
		// Close the handle
	}
}
vector< vector<BYTE> > fat;
void printData(BYTE DATA[512]) { // In ra Bang
	int k = 0;
	cout << "      ";
	for (int i = 0; i < 16; i++) {
		cout << " ";
		if (i < 10)
			cout << i;
		else
			cout << char(i - 10 + 'A');

		if (i == 7)
			cout << " ";
		cout << " ";
	}
	cout << endl;

	for (int i = 0; i < 32; i++) {
		if (0 == k)
			cout << "0x000  ";
		else if (k <= 255)
			printf("0x0%x  ", k);
		else
			printf("0x%x  ", k);
		for (int j = 0; j < 16; j++) {
			int x = int(DATA[k]);
			if (x >= 0 && x < 16)
				cout << "0";

			printf("%x ", DATA[k++]);
			if (j == 7)
				cout << " ";
		}
		cout << endl;
	}
}
unsigned int toNumber(BYTE * DATA, int offset, int number) { // Chuyen du lieu o vi tri offset voi number phan tu thanh so
	unsigned int k = 0;
	memcpy(&k, DATA + offset, number);
	return k;
}
string toString(BYTE *DATA, int offset, int number) { // Chuyen du lieu o vi tri offset voi number phan tu thanh chuoi

	char * tmp = new char[number+1];
	memcpy(tmp, DATA + offset, number);
	string s = "";
	for (int i = 0; i < number; i++)
		if (tmp[i] != 0x00)
			s += tmp[i];

	return s;
}
void toByte(BYTE *Source, BYTE Des[], int offset, int number) { // Chuyen du lieu o vi tri offset voi number phan tu thanh BYTE
	memcpy(Des, Source + offset, number);
}
entry_temp readEntryTemp(BYTE data[]) {
	entry_temp entry;
	entry.offfset1C_4 = toString(data, 0x1C, 4);
	entry.offset1_10 = toString(data, 0x1, 10);
	entry.offsetE_12 = toString(data, 0xE, 12);
	return entry;
}
entry_main readEntryMain(BYTE data[]) {
	entry_main entry;
	entry.offset0_8 = toString(data, 0x0, 8);
	entry.offset8_3 = toString(data, 0x8, 3);
	entry.offset14_2 = toNumber(data, 0x14, 2);
	entry.offset1A_2 = toNumber(data, 0x1a, 2);
	entry.offset1C_4 = toNumber(data, 0x1c, 4);
	return entry;
}
bootSector readBootSector(BYTE sector[]) {
	bootSector bs;
	bs.fat_type = toString(sector, 0x52, 8);
	bs.byte_per_sec = toNumber(sector, 0xB, 2);
	bs.sec_per_clu = toNumber(sector, 0xD, 1);
	bs.sec_per_fat = toNumber(sector, 0x24, 4);
	bs.sec_extra_bootSec = toNumber(sector, 0xE, 2);
	bs.total_sector = toNumber(sector, 0x13, 2);
	if (bs.total_sector == 0){
		bs.total_sector = toNumber(sector, 0x20, 4);
	}
	bs.fat_num = toNumber(sector, 0x10, 1);
	bs.rdet_position = bs.sec_extra_bootSec + bs.fat_num*bs.sec_per_fat + (toNumber(sector, 0x2C, 4) - 2)*bs.sec_per_clu;
	bs.fat1_position = bs.sec_extra_bootSec;
	bs.data_position = bs.rdet_position;
	return bs;
}
int sector_index_from_cluster(bootSector bs, int cluster) {
	return bs.sec_extra_bootSec + bs.fat_num*bs.sec_per_fat + (cluster - 2)*bs.sec_per_clu;
}
int hexTodec(int byteHigh, int byteLow) {
	return byteHigh*16*16*16*16 + byteLow;
}
void printLine(BYTE line[32]) {
	cout << endl;
	for (int i=0; i<2; i++) {
		for (int j=0; j<16; j++) {
			printf("%x ", line[i*16+j]);
		}
		cout << endl;
	}
	cout << endl;
}

void solve(BYTE sector[],int offset_main, int offset, bootSector bs) {
	system("cls");
	ReadSect("\\\\.\\D:", sector, offset_main);
	//printData(sector);
	vector<entry> list;
	BYTE line[32];
	int k = 0;
	toByte(sector, line, offset + 32*(k++), 32);
	int offset_temp = offset_main;
	int offset_0 = offset;
	int choose;
	while(true) {
		if (line[0xB] == 0x0) {
			//printLine(line);
			break;
		}

		entry entry_new;
		while(line[0xB]==0x0F) {
			entry_temp temp = readEntryTemp(line);
			entry_new.list_temp.push_back(temp);
			if (offset/32 + k >= 16) {
				offset_main += 1;
				offset = 0;
				k = 0;
				ReadSect("\\\\.\\D:", sector, offset_main);
			}
			toByte(sector, line, offset + 32*(k++), 32);
		}
		entry_new.main = readEntryMain(line);
		list.push_back(entry_new);
		if (offset/32+k >= 16) {
			offset_main += 1;
			offset = 0;
			k = 0;
			ReadSect("\\\\.\\D:", sector, offset_main);
		}
		toByte(sector, line, offset + 32*(k++), 32);
	}
	int n = list.size();
	if (n == 0){
		cout << "========================" << endl;
		cout << "= This Folder is empty ="<< endl;
		cout << "========================" << endl;
		cout << endl;
		system("pause");
		system("cls");
		return;
	}
		
	for (int i=0; i<list.size(); i++)
		reverse(list[i].list_temp.begin(), list[i].list_temp.end());
	while(true) {
		
		for (int i=0; i<list.size(); i++) {
			cout << i << " .";
			string name_file = "";
			if (list[i].list_temp.size() == 0) {
				name_file = list[i].main.offset0_8;
				cout << name_file;
				for (int space=0; space <100 - name_file.length(); space++)
					cout << " ";
				cout << "| ";
				cout << list[i].main.offset1C_4 << " Byte" << endl;
				cout << "------------------------------------------------------------------------------------"<< endl;
				continue;
			}
			for (int j=0; j<list[i].list_temp.size(); j++) {
				name_file = name_file + list[i].list_temp[j].offset1_10 + list[i].list_temp[j].offsetE_12 + list[i].list_temp[j].offfset1C_4;
			}
			cout << name_file;
			for (int space=0; space <100 - name_file.length(); space++)
				cout << " ";
			cout << "| ";
			cout << list[i].main.offset1C_4 << " Byte";
			cout << endl << "------------------------------------------------------------------------------------"<< endl;
		}
		cout << endl << endl;
		cout << "========================" << endl;
		cout << "===== Enter choose =====" << endl;
		cout << "========================" << endl;
		cout << endl;
		cout << ">> ";
		cin >> choose;
		while(choose < -2 || choose > n) {
			cout << "========================" << endl;
			cout << "== Enter choose again ==" << endl;
			cout << "========================" << endl;
			cout << endl;
			cout << ">> ";
			cin >> choose;
		}
		cout << endl << endl;
		if(choose == -1){
			system("cls");
			return;
		}
			
		else if (choose ==-2){
			cout << endl;
			cout << "========================" << endl;
			cout << "== Closing program... ==" << endl;
			cout << "========================" << endl;
			exit(0);
		}
			
		else {
			int offset_cluster_choose = hexTodec(list[choose].main.offset14_2, list[choose].main.offset1A_2);
			int sector_index = sector_index_from_cluster(bs, offset_cluster_choose);
			if (list[choose].main.offset8_3 == "TXT") {
				system("cls");
				BYTE cluster[4];
				BYTE data_text[512];
				BYTE cluster_temp[512];
				int a = offset_cluster_choose/128;
				int b = offset_cluster_choose%128;
				ReadSect("\\\\.\\D:", cluster_temp, bs.fat1_position + a);
				toByte(cluster_temp, cluster, b*4,4);
				int p = toNumber(cluster, 0, 4);
				printf("%x\n", p);
				while(p != 0x0FFFFFFF){
					for (int ite=0; ite < bs.sec_per_clu; ite++){		
						ReadSect("\\\\.\\D:", data_text, sector_index++);
						string ans = toString(data_text, 0, 512);
						cout << ans;
					}
					a = p/128;
					b = p%128;
					ReadSect("\\\\.\\D:", cluster_temp, bs.fat1_position + a);
					toByte(cluster_temp, cluster, b*4, 4);
					sector_index = sector_index_from_cluster(bs, p);
					p = toNumber(cluster, 0, 4);	
				}
				for (int ite=0; ite<bs.sec_per_clu; ite++){		
					ReadSect("\\\\.\\D:", data_text, sector_index++);
					string ans = toString(data_text, 0, 512);
					cout << ans;
				}
				cout << "cc" << endl;
				cout << endl;
				system("pause");
				system("cls");
				
				
			}
			else if (list[choose].main.offset8_3 != "   "){
				cout << "----- Use a dedicated program to read the content of the "<< list[choose].main.offset8_3 << " file -------" << endl << endl;
				system("pause");
				system("cls");
			}
			else 
				solve(sector, sector_index, 4*16, bs);
		}
		cout << endl << endl;
	}




}

int main() {
	int iterator = 0;
	BYTE sector[512];
	ReadSect("\\\\.\\D:", sector, 0);
	bootSector bs = readBootSector(sector);
	cout <<"===========================================================" << endl;
	cout <<"================ FILE SYSTEM READ =========================" << endl;
	cout <<"===========================================================" << endl;
	cout <<"       Fat type: "  << bs.fat_type << endl;
	printf("       Byte per sector: %d\n", bs.byte_per_sec);
	printf("       Sector per cluster: %d\n", bs.sec_per_clu);
	printf("       Sector per fat: %d\n", bs.sec_per_fat);
	printf("       Sector Boot Sector: %d\n", bs.sec_extra_bootSec);
	printf("       Sector data position : %d\n", bs.data_position);
	printf("       Total sector: %d\n", bs.total_sector);
	printf("       Sector fat positon : %d\n", bs.fat1_position);
	printf("       The number of fat: %d\n", bs.fat_num);
	printf("       Rdet position: %d\n", bs.rdet_position);
	cout <<"===========================================================" << endl;
	cout <<"===========================================================" << endl;
	cout <<"===========================================================" << endl;
	cout << endl << endl;
	
	cout <<"======================= Instruction for user =========================" << endl;
	cout <<"=== Press number from 0 to n to access the folder or file item =======" << endl;
	cout <<"============== Press -1 to BACK or -2 to EXIT program ================" << endl;
	cout <<"======================================================================" << endl;
	cout << endl;
	system("pause");
	// read FAT
	
//	for (int i=0; i<fat.size(); i++){
//		reverse(fat[i].begin(), fat[i].end());
//	}
	
	
//	for (int i=0; i<fat.size(); i++){
//		for (int j=0; j<fat[i].size(); j++){
//			printf("%x ", fat[i][j]);
//		}
//		cout << endl;
//	}
	
	
	


	
	solve(sector, bs.rdet_position, 8*16, bs);

	



}
