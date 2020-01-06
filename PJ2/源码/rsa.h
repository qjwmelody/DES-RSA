#include<iostream>
#include<stdlib.h>

extern "C" {
	#include "miracl.h"
	#include "mirdef.h"
}
using namespace std;

//������ģ
void pow_mod(big a, big b, big n, big z) {
	big save_a = mirvar(0);
	big save_b = mirvar(0);
	copy(a, save_a);
	copy(b, save_b);

	big zero = mirvar(0);
	while (mr_compare(b,zero)) {
		divide(a, n, n);//a=a%n��ֹ���
		if (!subdivisible(b, 2)) {//������ܳ�����Ҳ����ĩλ��1
			multiply(z, a, z);
			divide(z, n, n);//z=z*a%n
		}
		sftbit(b, -1, b);//����һλ
		multiply(a, a, a);
		divide(a, n, n);//a=a*a%n
	}
	copy(save_a, a);
	copy(save_b, b);
}

//���Լ�⣨Miller-Rabin��ⷨ��
bool is_prime(big num, int times) {
	big zero = mirvar(0);
	big one = mirvar(1);
	big two = mirvar(2);
	if (!mr_compare(num, one))//1��������
		return false;
	if (!mr_compare(num, two)) //2������
		return true;
	big t = mirvar(0);
	decr(num, 1, t);//t=num-1

	if (!subdivisible(t, 2))//ĩλ��1������������֮ǰ����ż����������
		return false;

	//	������k��m��ʹ��n-1=2^k*m
	//������ͳ��n-1��ĩβ�м���0���͵õ�k
	int k=0;
	while (mr_compare(t, one)) {//���t>0
		if (subdivisible(t, 2))
			break;
		k++;
		sftbit(t, -1, t);//����
	}

	big temp = mirvar(0);
	big m = mirvar(0);
	expb2(k, temp);//temp=2^k
	divide(t, temp, m);//m=t/temp; t=t mod temp
	decr(num, 1, t);//t=num-1���ָ�t��ֵ

	//ѭ��,����times��
	for (int i = 1; i <= times; i++) {
		big a = mirvar(0);
		big b = mirvar(1);
		bigrand(t, a);
		pow_mod(a, m, num, b);//b=a^m mod num
		divide(b, num, num);//b=b%num
		if (!mr_compare(b ,one))//b==1, ����Ϊ����
			continue;
		bool flag = true;
		//��������1<=j<k��a^(2^j*m) mod num!=-1
		for (int j = 1; j < k&&flag; j++) {
			if (b == t)
				flag = false;//��һ����ȣ�����������
			else {
				multiply(b, b, b);
				divide(b, num, num);
			}
		}
		if(flag)//���������ȣ�һ���Ǻ���
			return false;
	}
	return true;//ͨ�����в���
}

//�������һ������
void create_prime(int len, big p, int times) {
	bigdig(len, 2, p);
	while (!is_prime(p, times))
		bigdig(len, 2, p);
}


//�������
void my_sub(big a, big b, big z) {
	negify(b, b);//b=-b
	add(a, b, z);//z=a+b
	negify(b, b);//b=-b,��Ҫ�����������ı����ֵ
}


/*
//��չŷ������㷨���ݹ�ʵ�֣�
big ex_gcd(big a, big b, big &x, big &y) {
	big zero = mirvar(0);
	big one = mirvar(1);
	if (!mr_compare(b, zero)) {
		copy(one, x);
		copy(zero, y);
		return a;
	}
	big temp = mirvar(0);
	copy(a, temp);
	divide(a, b, b);//a=a%b
	big r = ex_gcd(b, a, x, y);
	copy(temp, a);
	big t = mirvar(0);
	copy(x, t);
	copy(y, x);
	copy(a, temp);//�ݴ�a��ֵ
	divide(a, b, a);//a=a/b
	multiply(a, y, a);//a=a*y
	negify(a, a);//a=-a
	add(t, a, y);
	copy(temp, a);
	return r;
}
*/

//��չŷ������㷨���ǵݹ�ʵ�֣�
big extended_Euclid(big a, big b, big &x, big &y) {
	big zero = mirvar(0);
	big one = mirvar(1);

	big save_b = mirvar(0);
	big save_a = mirvar(0);
	copy(b, save_b);
	copy(a, save_a);

	big x1, y1, x0, y0;
	x0 = mirvar(1);
	y0 = mirvar(0);
	x1 = mirvar(0);
	y1 = mirvar(1);

	copy(zero, x);
	copy(one, y);

	big r = mirvar(0);
	copy(a, r);//r�ݴ�a��ֵ
	divide(r, b, b);//r=a%b

	big a_r = mirvar(0);
	my_sub(a, r, a_r);
	big q = mirvar(0);
	copy(a_r, q);
	divide(q, b, q);//q=(a-r)/b

	while (mr_compare(r, zero)) {
		big q_multi_x1 = mirvar(0);
		big q_multi_y1 = mirvar(0);
		multiply(q, x1, q_multi_x1);
		multiply(q, y1, q_multi_y1);
		my_sub(x0, q_multi_x1, x);//x = x0 - q*x1;
		my_sub(y0, q_multi_y1, y);//y = y0 - q*y1

		copy(x1, x0);
		copy(y1, y0);
		copy(x, x1);
		copy(y, y1);

		copy(b, a);
		copy(r, b);
		copy(a, r);//�ݴ�a��ֵ
		divide(r, b, b);//r=a%b
		
		my_sub(a, r, a_r);
		copy(a_r, q);
		divide(q, b, q);//q=(a-r)/n
	}
	copy(save_b, b);
	copy(save_a, a);
	return b;
}
