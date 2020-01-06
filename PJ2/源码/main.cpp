#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fstream>
#include<string>
#include<time.h>
#include<bitset>
#include"const.h"
#include"rsa.h"
extern "C" {//c++��������c��
	#include "miracl.h"
	#include "mirdef.h"
}
using namespace std;

//ȫ�ֱ���
bitset<64> key_64;
bitset<48> key_48[8];

//ѭ������
bitset<28> leftshift(bitset<28> key, int shift) {
	bitset<28> temp = key;
	for (int i = 27; i >= 0; i--) {
		if (i - shift >= 0)
			key[i] = temp[i - shift];
		else
			key[i] = temp[i - shift + 28];
	}
	return key;
}

//��������Կ
void generate_keys() {
	bitset<56> key_56;
	bitset<28> C;
	bitset<28> D;
	bitset<48> key_48;
	//ȥ����ż���λ����64λ��Կ��Ϊ56λ
	for (int i = 0; i != 56; i++)
		key_56[i] = key_64[IPC_table[i]];

	for (int i = 0; i != 28; i++) {
		C[i] = key_56[i];
		D[i] = key_56[i + 28];
	}
	for (int round = 0; round != 8; round++) {
		//�ֱ�����
		int shift = LS_table[round];
		C = leftshift(C, shift);
		D = leftshift(D, shift);
		//������ϳ�56λ
		for (int i = 0; i != 28; i++) {
			key_56[i] = C[i];
			key_56[i + 28] = D[i];
		}
		//�û�Ϊ48λ���õ���������Կ
		for (int i = 0; i != 48; i++)
			key_48[round] = key_56[PC_table[i]];
	}
}


//S��
void S_func(bitset<48> a) {
	//48λ�ֳ�8*6�ľ���
	int group[8][6];
	for (int i = 0; i != 48; i++)
		group[i / 6][i % 6] = a[i];
	for (int i = 0; i != 8; i++) {
		int row = 2 * group[i][0] + group[i][5];
		int col = 8 * group[i][1] + 4 * group[i][2] + 2 * group[i][3] + group[i][4];
		int value = S_table[i][row][col];
		//valueת����0/1��
		int bin[4];
		bin[0] = value / 8;
		bin[1] = value % 8 / 4;
		bin[2] = value % 4 / 2;
		bin[3] = value % 2;
	}
	for (int i = 0; i != 32; i++)
		a[i] = group[i / 4][i % 4];
}

//�ֺ���F
bitset<32> F_func(bitset<32> R, bitset<48>key) {
	bitset<32> R2;
	//��չ�û�
	bitset<48> expandedR;
	for (int i = 0; i != 48; i++)
		expandedR[i] = R[E_table[i]];
	//���
	expandedR = expandedR^key;
	//S��
	S_func(expandedR);
	//P�û�
	for (int i = 0; i != 32; i++)
		R2[i] = expandedR[P_table[i]];
	return R2;
}


//���ܽ���
bitset<64> enc_dec(bitset<64> input, bool en_de) {
	bitset<32> L;
	bitset<32> R;
	bitset<32> temp;
	bitset<64> output;

	//IP�±��û�
	bitset<64> input_t;
	for (int i = 0; i != 64; i++)
		input_t[i] = input[IP_table[i]];
	input = input_t;
	//���ߵ�32λ�ֿ�
	for (int i = 0; i != 32; i++) {
		L[i] = input[i];
		R[i] = input[i + 32];
	}
	//����8�ε���
	for (int round = 0; round != 8; round++) {
		if (en_de == 0) {
			temp = R;
			R = L^F_func(R, key_48[round]);
			L = temp;
		}
		else {
			temp = L;
			L = R^F_func(L, key_48[7 - round]);
			R = temp;
		}
	}
	//�ϲ�
	for (int i = 0; i != 32; i++) {
		output[i] = L[i];
		output[i + 32] = R[i];
	}
	//IP-1�û�
	bitset<64> output_t;
	for (int i = 0; i != 64; i++)
		output_t[i] = output[IPR_table[i]];
	return output_t;
}

bitset<64> StringToBitset(string s) {
	bitset<64> bits;
	for (int i = 0; i != 64; i++)
		bits[i] = s[i] - '0';
	return bits;
}

string BitsetToString(bitset<64> b) {
	bitset<64> b2;
	for (int i = 0; i != 64; i++)
		b2[i] = b[63 - i];
	return b2.to_string();
}



int main() {
	//Ŀ¼
	cout << endl;
	cout << "---------------------------8��DES�ļ�����-----------------------------------" << endl;
	printf("����ģʽ��%35c����Ϊ0/����Ϊ1\n", ' ');
	printf("��Կ��%39c����Ϊ64λ�Ķ�����0/1��\n", ' ');
	printf("�����ļ���ַ��%31c���Ե�ַ/��Ե�ַ\n", ' ');
	printf("����ļ���ַ��%31c���Ե�ַ/��Ե�ַ\n", ' ');
	cout << "----------------------------------------------------------------------------" << endl;
	cout << endl;

	//RSA��������
	cout << "������������RSA�㷨����������Կ��..." << endl;
	cout << "���ɳɹ���" << endl;
	miracl *mip = mirsys(2048, 2);//��ʼ��
	mip->IOBASE = 2;//�������Ϊ������
	big p, q, n, fn, e, d;
	big zero, one, minus_one;
	p = mirvar(0);
	q = mirvar(0);
	n = mirvar(0);
	fn = mirvar(0);
	e = mirvar(0);
	d = mirvar(0);
	zero = mirvar(0);
	one = mirvar(1);
	minus_one = mirvar(-1);

	//���p��q
	do {
		create_prime(64, p, 10);
		create_prime(64, q, 10);
		if (mr_compare(p, q))//�����Ƿ����
			break;
	} while (1);
	printf("p=");
	otnum(p, stdout);
	printf("q=");
	otnum(q, stdout);

	//���n��fn
	multiply(p, q, n);//n=p*q
	add(p, minus_one, p);
	add(q, minus_one, q);
	multiply(p, q, fn);//fn=(p-1)*(q-1)
	printf("n=");
	otnum(n, stdout);
	printf("fn=");
	otnum(fn, stdout);

	//���d��e
	do {
		bigrand(fn, d);//�������С��fn������
		if (egcd(d, fn, one) == 1)
			break;
	} while (1);
	printf("d=");
	otnum(d, stdout);
	big x = mirvar(1);
	big y = mirvar(1);
	extended_Euclid(d, fn, x, y);
	if (mr_compare(x, zero) < 0)
		add(x, fn, x);
	copy(x, e);
	printf("e=");
	otnum(e, stdout);
	cout << endl;


	cout << "��ѡ����ģʽ��";
	bool m;
	cin >> m;

	//����ģʽ
	if (m == 0) {
		cout << "�������ʼ��Կ��";
		string k;
		cin >> k;
		if (k.length() != 64) {
			cout << "��Կ��������" << endl << endl;
			system("pause");
			return 1;
		}
		key_64 = StringToBitset(k);
		generate_keys();

		//ʹ��RSA�Գ�ʼ��Կ���м���
		//���Ȱ�stringת����big����
		big big_key1 = mirvar(0);
		char s[129] = { 0 };
		for (int i = 0; i != 64; i++)
			s[i] = k[i];
		cinstr(big_key1, s);
		printf("��ʼ��Կ��");
		otnum(big_key1, stdout);
		//����
		big enc = mirvar(1);
		pow_mod(big_key1, e, n, enc);
		printf("���ܺ�");
		otnum(enc, stdout);


		cout << "�����������ļ�����.txt����";
		string filename1;
		cin >> filename1;
		ifstream in(filename1, ios::binary | ios::in);
		cout << "�����������ļ�����.txt����";
		string filename2;
		cin >> filename2;
		ofstream out(filename2, ios::binary | ios::out);
		cout << endl;
		if (!in || !out) {
			cout << "�ļ���ʧ�ܣ�" << endl << endl;
			system("pause");
			return 1;
		}
		else
			cout << "�ѳɹ����ļ���" << endl;

		//�Ѽ��ܺ����Կ����������ļ���
		string big_key2;
		cotstr(enc, s);
		big_key2 = s;
		out << big_key2 << endl;

		//IV
		char IV[64];
		srand(time(NULL));   //ʹ�õ�ǰʱ����Ϊ����
		for (int i = 0; i < 64; i++) {
			IV[i] = (char)(rand() % 2 + '0');
			//rand()�����������Ȼ��ģ2����ǿ��ת������
		}

		bitset<64> IV2;
		IV2 = StringToBitset(IV);

		//��IV���ܣ�����������ļ�
		bitset<64> IV3 = enc_dec(IV2, 0);
		for (int i = 0; i < 64; i++)
			out << IV3[i];
		out << endl;

		//���һ�鲻��64λ���0���������8λ��ʾ���ռ�е�λ��
		string si;
		in >> si;
		int count = si.length() % 64;
		count = 56 - count;
		if (count < 0)
			count += 64;
		char b = '0';
		for (int i = 0; i != count; i++)
			si += b;
		//���8λ
		for (int i = 7; i >= 0; i--) {
			char last = count % 2 + 48;
			si += last;
			count /= 2;
		}
		int len = si.length();
		cout << "��ʼ����..." << endl;
		cout << filename1 << "-->" << filename2 << endl;
		bitset<64> input;
		bitset<64> input2;
		bitset<64> output;
		for (int i = 0; i != len; i = i + 64) {
			//��64λΪһ����ü����㷨
			string substri = si.substr(i, 64);
			input = StringToBitset(substri);
			input2 = IV2^input;
			output = enc_dec(input2, 0);
			IV2 = output;
			string so = BitsetToString(output);
			out << so;
		}
		cout << "���ܳɹ���";
		cout << endl << endl;
		in.close();
		out.close();
	}
	
	//����ģʽ
	else {
		cout << "�����������ļ�����.txt����";
		string filename1;
		cin >> filename1;
		ifstream in(filename1, ios::binary | ios::in);
		cout << "�����������ļ�����.txt����";
		string filename2;
		cin >> filename2;
		ofstream out(filename2, ios::binary | ios::out);
		cout << endl;
		if (!in || !out) {
			cout << "�ļ���ʧ�ܣ�" << endl << endl;
			system("pause");
			return 1;
		}
		else
			cout << "�ѳɹ����ļ���" << endl;
		
		//��ȡ������Կ
		string big_key3;
		in >> big_key3;
		int len7 = big_key3.length();
		char s7[129] = { 0 };
		for (int i = 0; i!=len7; i++)
			s7[i] = big_key3[i];
		big big_key4 = mirvar(0);
		cinstr(big_key4, s7);

		big dec = mirvar(1);
		pow_mod(big_key4, d, n, dec);
		printf("���ܺ����Կ��");
		otnum(dec, stdout);

		string big_key5;
		cotstr(dec, s7);
		big_key5 = s7;

		key_64 = StringToBitset(big_key5);
		generate_keys();

		//��ȡIV��IV����һ�У���������
		string IV;
		in >> IV;
		bitset<64> IV2;
		IV2 = StringToBitset(IV);
		IV2 = enc_dec(IV2, 1);

		//��ȡ����
		string si;
		in >> si;
		int len = si.length();
		cout << "��ʼ����..." << endl;
		cout << filename1 << "-->" << filename2 << endl;

		bitset<64> input;
		bitset<64> input2;
		bitset<64> output;
		//��¼����λ��
		int count = 0;
		string str = si.substr(len - 64, 64);
		input = StringToBitset(str);
		output = enc_dec(input, 1);
		if (len == 64)
			output = output^IV2;
		else {
			string before_str = si.substr(len - 128, 64);
			input2 = StringToBitset(before_str);
			output = output^input2;//�ٺ�ǰһ���������õ�ԭ��
		}
		string so = BitsetToString(output);

		int quanzhong = 1;
		for (int j = 56; j <= 63; j++) {
			int value = so[j] - 48;
			count += value*quanzhong;
			quanzhong *= 2;
		}

		for (int i = 0; i <= len - 64; i += 64) {
			string substri = si.substr(i, 64);
			input = StringToBitset(substri);
			output = enc_dec(input, 1);//�Ƚ���
			if (i == 0)
				output = output^IV2;
			else {
				string before_str = si.substr(i - 64, 64);
				input2 = StringToBitset(before_str);
				output = output^input2;//�ٺ�ǰһ���������õ�ԭ��
			}
			string so = BitsetToString(output);

			//��������һ��
			if (i == len - 64) {
				if (count == 56)//�պ���64����������û������������Ҫ���
					continue;
				else if (count < 56) {//���ԭ�����һ�����λ��>=8��ֱ�����ǰ���λ
					for (int j = 0; j != 56 - count; j++)
						out << so[j];
				}
				else {//���ԭ�����һ�����λ��<8,��һ��Ҳ�����
					continue;
				}
			}
			else if (count > 56 && i == len - 128) {
				for (int j = 0; j != 64 - (count - 56); j++)
					out << so[j];
			} //�����������������������ڶ����ǰ�沿��		
			else
				out << so;
		}
		cout << "���ܳɹ���" << endl << endl;
		in.close();
		out.close();
	}

	system("pause");
 	return 0;
}