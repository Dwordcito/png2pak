/******************************************************************
spriteIndex * 4 = spriteStart -> byte donde arranca la cabecera del sprite
spriteStart = m_iTotalFrame->cantidad de frames que tiene el sprite
spriteStart + 4 = Tamaño de la imagen
spriteStart + 8 = Width de la imagen
spriteStart + 12 = Height de la imagen
spriteStart + 16+(12*m_iTotalFrame) = Empieza la imagen
******************************************************************/

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <winsock.h>

using namespace std;

vector<string> get_all_files_names_within_folder(string folder)
{
	vector<string> names;
	char search_path[600];
	string str;
	sprintf(search_path, "%s/*.*", folder.c_str());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				str = fd.cFileName;
				if (str.find(".bmp")) {
					names.push_back(str);
				}
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

DWORD getFileSize(string filename) {
	FILE *p_file = NULL;
	p_file = fopen(filename.c_str(), "rb");
	fseek(p_file, 0, SEEK_END);
	DWORD size = ftell(p_file);
	fclose(p_file);
	return size;
}

int getHeight(string ruta) {
	std::ifstream in(ruta);
	unsigned int height;

	in.seekg(20);
	in.read((char *)&height, 4);

	height = ntohl(height);

	return height;
}

int getWidth(string ruta) {
	std::ifstream in(ruta);
	unsigned int width;

	in.seekg(16);
	in.read((char *)&width, 4);

	width = ntohl(width);

	return width;
}

int main(int argc, char * argv[]) {
	DWORD dwSize = 0, dwFileSize;
	FILE * pFilePNG, *pFilePAK;
	char *pBuffer, *cp, *initial;
	int *ip;

	vector <string> archivos = get_all_files_names_within_folder(argv[1]);

	for (UINT i = 0; i < archivos.size(); i++) {
		dwSize += getFileSize(argv[1] + ('\\' + archivos[i]));
	}
	dwSize += 4 * (archivos.size() / 2);
	dwSize += 16 * (archivos.size() / 2);

	pBuffer = new char[dwSize + 1];
	ZeroMemory(pBuffer, sizeof(pBuffer));

	initial = cp = (char *)pBuffer;

	//carga puntos de inicio de los sprites
	for (UINT j = 0; j < archivos.size(); j += 2) {
		ip = (int *)cp;
		if (j == 0) *ip = 4 * (archivos.size() / 2);
		else *ip = getFileSize(argv[1] + ('\\' + archivos[j - 2])) + 16 + getFileSize(argv[1] + ('\\' + archivos[j - 1]));
		cp += 4;
	}

	/*cp = initial;

	//carga cantidad de frames
	cp += 4*(archivos.size() / 2);*/
	for (UINT j = 0; j < archivos.size(); j += 2) {
		ip = (int *)cp;
		*ip = getFileSize(argv[1] + ('\\' + archivos[j + 1])) / 12;
		cp += getFileSize(argv[1] + ('\\' + archivos[j])) + 16 + getFileSize(argv[1] + ('\\' + archivos[j + 1]));
	}

	cp = initial;

	//tamaño de la imagen
	cp += 4 * (archivos.size() / 2) + 4;
	for (UINT j = 0; j < archivos.size(); j += 2) {
		ip = (int *)cp;
		*ip = getFileSize(argv[1] + ('\\' + archivos[j]));
		cp += getFileSize(argv[1] + ('\\' + archivos[j])) + 16 + getFileSize(argv[1] + ('\\' + archivos[j + 1]));
	}
	cp = initial;

	//width
	cp += 4 * (archivos.size() / 2) + 8;
	for (UINT j = 0; j < archivos.size(); j += 2) {
		ip = (int *)cp;
		*ip = getWidth(argv[1] + ('\\' + archivos[j]));
		cp += getFileSize(argv[1] + ('\\' + archivos[j])) + 16 + getFileSize(argv[1] + ('\\' + archivos[j + 1]));
	}
	cp = initial;

	//height
	cp += 4 * (archivos.size() / 2) + 12;
	for (UINT j = 0; j < archivos.size(); j += 2) {
		ip = (int *)cp;
		*ip = getHeight(argv[1] + ('\\' + archivos[j]));
		cp += getFileSize(argv[1] + ('\\' + archivos[j])) + 16 + getFileSize(argv[1] + ('\\' + archivos[j + 1]));
	}
	cp = initial;

	//imagen y framesizes
	cp += 4 * (archivos.size() / 2) + 16;
	for (UINT j = 0; j < archivos.size(); j += 2) {
		dwFileSize = getFileSize(argv[1] + ('\\' + archivos[j + 1]));
		pFilePNG = fopen((argv[1] + ('\\' + archivos[j + 1])).c_str(), "rb");
		fread(cp, dwFileSize, 1, pFilePNG);
		fclose(pFilePNG);
		cp += dwFileSize;
		dwFileSize = getFileSize(argv[1] + ('\\' + archivos[j]));
		pFilePNG = fopen((argv[1] + ('\\' + archivos[j])).c_str(), "rb");
		fread(cp, dwFileSize, 1, pFilePNG);
		fclose(pFilePNG);
		cp += dwFileSize + 16;
	}
	char cNewName[256];
	ZeroMemory(cNewName, sizeof(cNewName));
	sprintf(cNewName, "%s\\woot.dat", argv[1]);
	pFilePAK = fopen(cNewName, "wb");
	fwrite(pBuffer, sizeof(char), dwSize, pFilePAK);
	fclose(pFilePAK);
	return dwSize;
}