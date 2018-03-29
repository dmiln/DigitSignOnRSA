#include <iostream>
#include <Windows.h>
#include <fstream>
#include "big.h"
#include "flash.h"
#include <time.h>
#include <string>

//using namespace::std;

extern "C"
{
#include "miracl.h"
}


Miracl precision = 300;   /* number of ints per ZZn */
Big genE(Big);
Big genD(Big, Big);
Big* generation(int, int);
Big hashFun(char*, int);
char* makeMessage(int);
Big Enc(Big &, Big&, Big &);
Big Dec(Big &, Big&, Big &);
int readFile(char *);

sha512 ptr;

struct bits
{
	unsigned char b1 : 1;
	unsigned char b2 : 1;
	unsigned char b3 : 1;
	unsigned char b4 : 1;
	unsigned char b5 : 1;
	unsigned char b6 : 1;
	unsigned char b7 : 1;
	unsigned char b8 : 1;
};
union byte1
{
	unsigned char ch;
	struct bits bts;
};

/*	Функция генерации числа N
	@k - размерность q
	@l - размерность l
	Функция возвращает массив, хранящий число N и функцию эйлера этого числа
*/
Big* generation(int k, int l) {
	gprime(10000);
	Big N, eul;
	big p = mirvar(1), q = mirvar(1);
	Big* arr = new Big[2];
	while (!isprime(p)) {
		bigbits(k, p);
	}
	while (!isprime(q)) {
		bigbits(l, q);
	}
	N = (Big)p * (Big)q;
	eul = ((Big)p - 1)*((Big)q - 1);
	cout << "p = " << (Big)p << endl << endl << "q = " << (Big)q << endl << endl;
	arr[0] = N;
	arr[1] = eul;
	return arr;
}

//генерация e, на вход приходит функция эйлера
Big genE(Big eul) {
	int x = bits(eul);//число бит
	Big e;
	big tmp = mirvar(1);
	bigbits(x/6, tmp);
	for (Big i = tmp; i > 1; i=i-1) {
		e = gcd(i, eul);
		if (e == 1) {
			e = i;
			break;
		}
	}
	return e;
}
//генерируем d с помощью расширенного алг-ма Евклида
Big genD(Big e, Big eul) {
	Big d = 1;
	//big d = mirvar(1); 
	d = inverse(e, eul);
	//xgcd(e.getbig(), eul.getbig(), d, d, d);
	return d;
}

Big hashFun(char * text,int bytes)
{
	char hash[64];//кол-во байт на хэш
	Big h;
	while (*text) 
		shs512_process(&ptr, *text++);
	shs512_hash(&ptr, hash);
	h = from_binary(bytes, hash);
	return h;
}

char* makeMessage(int len)
{
	char* text = new char[len];
	for (int i = 0; i < len; i++)
		*(text + i) = rand() % 256;
	return text;
}

Big Enc(Big &m, Big&d, Big &N)
{
	return pow(m, d, N);
}

Big Dec(Big &s, Big&e, Big &N)
{
	return pow(s, e, N);
}

int readFile(char *str)  {
	ifstream file(str);
	char a;
	int count = 1;
	while (file.get(a))
	{
		if (a == '\n') {
			count++;
		}
	}
	file.close();
	return count;
}

int countBytes(int bitsCount)
{
	return  bitsCount < 8 ? 1 : bitsCount % 8 == 0 ? bitsCount / 8 : 1 + bitsCount / 8;
}

void hashArray(char *str1, char *str2, char *str3, Big N, Big e, Big d, int bytes) {
	Big hashA;
	byte1 b;
	int *bitsArray;
	bitsArray = new int[bytes * 8];
	char* mass;

	string s; //сюда будем класть считанные строки
	ifstream file(str1);
	ofstream write, binary;
	write.open(str2, ios_base::ate);
	for (int i = 0; i < bytes * 8; i++)
		bitsArray[i]=0;
	//memset(bitsArray, 0, sizeof(int)*8);//заполнение массива 0
	while (getline(file, s)) { // пока не достигнут конец файла класть очередную строку в переменную (s)
		mass = new char[bytes];
		int c = s.length();
		char str[100];
		strcpy_s(str, s.c_str());
		hashA = hashFun(str, bytes);
		write << /*s << " " <<*/ hashA << '\n';
		int hashLength = countBytes(bits(hashA));
		to_binary(hashA, hashLength, mass, false);
		for (int i = hashLength - 1, k = 0; i >= 0; i--, k++)
		{
			b.ch = mass[i];
			bitsArray[k * 8] += b.bts.b1;
			bitsArray[k * 8 + 1] += b.bts.b2;
			bitsArray[k * 8 + 2] += b.bts.b3;
			bitsArray[k * 8 + 3] += b.bts.b4;
			bitsArray[k * 8 + 4] += b.bts.b5;
			bitsArray[k * 8 + 5] += b.bts.b6;
			bitsArray[k * 8 + 6] += b.bts.b7;
			bitsArray[k * 8 + 7] += b.bts.b8;
		}
		delete[]mass;
	}

	binary.open(str3, ios_base::out);
	for (int i = 0; i < bytes * 8; i++) {
		binary << bitsArray[i] << '\n';
	}

	delete[] bitsArray;
	binary.close();
	write.close();
	file.close();
}

int main() {
	srand(time(NULL));
	SYSTEMTIME st, fn;
	Big e = 1, d = 1, S = 1, N = 1, hash1=1, hash2=1;
	bigbits(97, hash1.getbig());
	Big* mass = new Big[2];
	mass = generation(260, 260);
	e = genE(mass[1]);
	d = genD(e, mass[1]);
	N = mass[0];
	cout << "N=" << N << endl << endl;
	cout << "euler=" << mass[1] << endl << endl;
	cout << "e=" << e << endl << endl;
	cout << "d=" << d << endl << endl;

	shs512_init(&ptr);
	char* sms = makeMessage(rand()%100);
	cout << "Message: \n";
	cout << sms << endl << endl;

	hash1 = hashFun(sms, 64);
	delete[]sms;
	cout << "hash1=" << hash1 << endl << endl;

	S = Enc(hash1, d, N);
	hash2 = Dec(S, e, N);
	cout << "hash2=" << hash2 << endl << endl;
	
	if (hash1 == hash2) {
		cout << "Message decoded correctly!!!" << endl;
	}
	else {
		cout << "Message not correctly!!!" << endl;
	}


	GetLocalTime(&st);
	hashArray("TextFiles/dictionary.txt", "TextFiles/Hash.txt","TextFiles/binary.txt", N, e, d, 64);
	GetLocalTime(&fn);
	cout << "Time " << (fn.wMinute *60 *1000 + fn.wSecond *1000 + fn.wMilliseconds) - (st.wMinute  *60 * 1000 + st.wSecond * 1000) + st.wMilliseconds << endl;


	cout << "all saved" << endl;
	getchar();
	return 0;
}