#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<fstream>
#include<bitset>
#include<string>
#include"const.h"
using namespace std;


//全局变量
bitset<64> key_64;
bitset<48> key_48[8];

//循环左移
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

//生成子密钥
void generate_keys() {
	bitset<56> key_56;
	bitset<28> C;
	bitset<28> D;
	bitset<48> key_48;
	//去掉奇偶标记位，将64位密钥变为56位
	for (int i = 0; i != 56; i++) 
		key_56[i] = key_64[IPC_table[i]];

	for (int i = 0; i != 28; i++) {
		C[i] = key_56[i];
		D[i] = key_56[i + 28];
	}
	for (int round = 0; round != 8; round++) {
		//分别左移
		int shift = LS_table[round];
		C = leftshift(C, shift);
		D = leftshift(D, shift);
		//重新组合成56位
		for (int i = 0; i != 28; i++) {
			key_56[i] = C[i];
			key_56[i + 28] = D[i];
		}
		//置换为48位，得到该轮子密钥
		for (int i = 0; i != 48; i++) 
			key_48[round] = key_56[PC_table[i]];
	}
}


//S盒
void S_func(bitset<48> a) {
	//48位分成8*6的矩阵
	int group[8][6];
	for (int i = 0; i != 48; i++)
		group[i / 6][i % 6] = a[i];
	for (int i = 0; i != 8; i++) {
		int row = 2 * group[i][0] + group[i][5];
		int col = 8 * group[i][1] + 4 * group[i][2] + 2 * group[i][3] + group[i][4];
		int value = S_table[i][row][col];
		//value转换成0/1串
		int bin[4];
		bin[0] = value / 8;
		bin[1] = value % 8 / 4;
		bin[2] = value % 4 / 2;
		bin[3] = value % 2;
	}
	for (int i = 0; i != 32; i++)
		a[i] = group[i / 4][i % 4];
}

//轮函数F
bitset<32> F_func(bitset<32> R, bitset<48>key) {
	bitset<32> R2;
	//扩展置换
	bitset<48> expandedR;
	for (int i = 0; i != 48; i++) 
		expandedR[i] = R[E_table[i]];
	//异或
	expandedR = expandedR^key;
	//S盒
	S_func(expandedR);
	//P置换
	for (int i = 0; i != 32; i++)
		R2[i] = expandedR[P_table[i]];
	return R2;
}


//加密解密
bitset<64> enc_dec(bitset<64> input, bool en_de) {
	bitset<32> L;
	bitset<32> R;
	bitset<32> temp;
	bitset<64> output;

	//IP下标置换
	bitset<64> input_t;
	for (int i = 0; i != 64; i++)
		input_t[i] = input[IP_table[i]];
	input = input_t;
	//按高低32位分开
	for (int i = 0; i != 32; i++) {
		L[i] = input[i];
		R[i] = input[i + 32];
	}
	//进行8次迭代
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
	//合并
	for (int i = 0; i != 32; i++) {
		output[i] = L[i];
		output[i + 32] = R[i];
	}
	//IP-1置换
	bitset<64> output_t;
	for (int i = 0; i != 64; i++)
		output_t[i] = output[IPR_table[i]];
	return output_t;
}

bitset<64> StringToBitset(string s) {
	bitset<64> bits;
	for (int i = 0; i != 64; i++) 
		bits[i] = s[ i] - '0';
	return bits;
}

string BitsetToString(bitset<64> b) {
	bitset<64> b2;
	for (int i = 0; i != 64; i++) 
		b2[i] = b[63 - i];
	return b2.to_string();
}

int main() {	
	cout << endl;
	cout << "---------------------------8轮DES文件加密-----------------------------------" << endl;
	printf("密钥：%39c长度为64位的二进制0/1串\n",' ');
	printf("工作模式：%35c加密为0/解密为1\n", ' ');
	printf("输入文件地址：%31c绝对地址/相对地址\n",' ');
	printf("输出文件地址：%31c可指定文件名，绝对地址/相对地址\n",' ');
	cout << "----------------------------------------------------------------------------" << endl;
	cout << endl;

	cout << "请输入密钥：";
	string k;
	cin >> k;
	if (k.length() != 64) {
		cout << "密钥长度有误！" << endl << endl;
		system("pause");
		return 1;
	}
	key_64 = StringToBitset(k);
	generate_keys();

	cout << "请选择工作模式：";
	bool m;
	cin >> m;
	if (m == 0) {
		//加密	
		cout << "请输入明文文件名（.txt）：";
		string filename1;
		cin >> filename1;
		ifstream in(filename1, ios::binary | ios::in);
		cout << "请输入密文文件名（.txt）：";
		string filename2;
		cin >> filename2;
		ofstream out(filename2, ios::binary | ios::out);
		cout << endl;
		if (!in || !out) {
			cout << "文件打开失败！" << endl << endl;
			system("pause");
			return 1;
		}	
		else
			cout << "已成功打开文件！" << endl;

		char IV[64];
		srand(time(NULL));   //使用当前时间作为种子
		for (int i = 0; i<64; i++) {
			IV[i] = (char)(rand() % 2 + '0');
			//rand()随机生成数字然后模2，再强制转换类型
		}

		bitset<64> IV2;
		IV2 = StringToBitset(IV);

		//给IV加密，输出到密文文件
		bitset<64> IV3 = enc_dec(IV2, 0);
		for (int i = 0; i < 64; i++)
			out << IV3[i];
		out << endl;


		//最后一组不满64位填充0，并且最后8位表示填充占有的位数
		string si;
		in >> si;
		int count = si.length() % 64;
		count = 56 - count;
		if (count <0)
			count += 64;
		char b = '0';
		for (int i = 0; i != count; i++)
			si += b;
		//最后8位
		for (int i = 7; i >= 0; i--) {
			char last = count % 2+48;
			si += last;
			count /= 2;
		}	
		int len = si.length();
		cout << "开始加密..." << endl;
		cout << filename1 << "-->" << filename2 << endl;
		bitset<64> input;
		bitset<64> input2;
		bitset<64> output;
		for (int i = 0; i != len ; i = i + 64) {
			//以64位为一组调用加密算法
			string substri = si.substr(i, 64);
			input = StringToBitset(substri);
			input2 = IV2^input;
			output = enc_dec(input2, 0);
			IV2 = output;
			string so = BitsetToString(output);
			out << so;
		}
		cout << "加密成功！";
		cout << endl << endl;
		in.close();
		out.close();		
	}

	else {
		//解密
		cout << "请输入密文文件名（.txt）：";
		string filename1;
		cin >> filename1;
		ifstream in(filename1, ios::binary | ios::in);
		cout << "请输入明文文件名（.txt）：";
		string filename2;
		cin >> filename2;
		ofstream out(filename2, ios::binary | ios::out);
		cout << endl;
		if (!in || !out) {
			cout << "文件打开失败！" << endl << endl;
			system("pause");
			return 1;
		}		
		else
			cout << "已成功打开文件！" << endl;		

		//读取IV（IV单独一行），并解密
		string IV;
		in >> IV;
		bitset<64> IV2;
		IV2 = StringToBitset(IV);
		IV2 = enc_dec(IV2, 1);

		//读取密文
		string si;
		in >> si;
		int len = si.length();
		cout << "开始解密..." << endl;
		cout << filename1 << "-->" << filename2 << endl;

		bitset<64> input;
		bitset<64> input2;
		bitset<64> output;
		//记录填充的位数
		int count=0;
		string str = si.substr(len - 64, 64);
		input = StringToBitset(str);
		output = enc_dec(input, 1);
		if (len == 64)
			output = output^IV2;
		else {
			string before_str = si.substr(len - 128, 64);
			input2 = StringToBitset(before_str);
			output = output^input2;//再和前一组密文异或得到原文
		}
		string so = BitsetToString(output);

		int quanzhong = 1;
		for (int j = 56; j <= 63; j++) {
			int value = so[j] - 48;
			count += value*quanzhong;
			quanzhong *= 2;
		}

		for (int i = 0; i <=len-64 ; i += 64) {
			string substri = si.substr(i, 64);
			input = StringToBitset(substri);		
			output = enc_dec(input, 1);//先解密
			if (i == 0)
				output = output^IV2;
			else {
				string before_str = si.substr(i - 64, 64);
				input2 = StringToBitset(before_str);
				output = output^input2;//再和前一组密文异或得到原文
			}		
			string so = BitsetToString(output);

			//如果是最后一组
			if (i == len - 64) {
				if (count == 56)//刚好是64的整数倍，没有填充过，不需要输出
					continue;
				else if (count < 56) {//如果原文最后一组空余位数>=8，直接输出前面的位
					for (int j = 0; j != 56 - count; j++) 
						out << so[ j];
				}
				else {//如果原文最后一组空余位数<8,这一组也不输出
					continue;
				}
			}
			else if (count > 56 && i == len - 128) {
				for (int j = 0; j != 64 - (count - 56); j++) 
					out << so[ j];
			} //如果是上面的情况，输出倒数第二组的前面部分		
			else
				out << so;
		}
		cout << "解密成功！" << endl<<endl;
		in.close();
		out.close();
	}	
	system("pause");
	return 0;
}