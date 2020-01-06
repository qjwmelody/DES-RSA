#include<iostream>
#include<stdlib.h>

extern "C" {
	#include "miracl.h"
	#include "mirdef.h"
}
using namespace std;

//快速幂模
void pow_mod(big a, big b, big n, big z) {
	big save_a = mirvar(0);
	big save_b = mirvar(0);
	copy(a, save_a);
	copy(b, save_b);

	big zero = mirvar(0);
	while (mr_compare(b,zero)) {
		divide(a, n, n);//a=a%n防止溢出
		if (!subdivisible(b, 2)) {//如果不能除尽，也就是末位是1
			multiply(z, a, z);
			divide(z, n, n);//z=z*a%n
		}
		sftbit(b, -1, b);//右移一位
		multiply(a, a, a);
		divide(a, n, n);//a=a*a%n
	}
	copy(save_a, a);
	copy(save_b, b);
}

//素性检测（Miller-Rabin检测法）
bool is_prime(big num, int times) {
	big zero = mirvar(0);
	big one = mirvar(1);
	big two = mirvar(2);
	if (!mr_compare(num, one))//1不是素数
		return false;
	if (!mr_compare(num, two)) //2是素数
		return true;
	big t = mirvar(0);
	decr(num, 1, t);//t=num-1

	if (!subdivisible(t, 2))//末位是1，即奇数，减之前就是偶数，非质数
		return false;

	//	下面求k和m，使得n-1=2^k*m
	//方法是统计n-1的末尾有几个0，就得到k
	int k=0;
	while (mr_compare(t, one)) {//如果t>0
		if (subdivisible(t, 2))
			break;
		k++;
		sftbit(t, -1, t);//右移
	}

	big temp = mirvar(0);
	big m = mirvar(0);
	expb2(k, temp);//temp=2^k
	divide(t, temp, m);//m=t/temp; t=t mod temp
	decr(num, 1, t);//t=num-1，恢复t的值

	//循环,测试times次
	for (int i = 1; i <= times; i++) {
		big a = mirvar(0);
		big b = mirvar(1);
		bigrand(t, a);
		pow_mod(a, m, num, b);//b=a^m mod num
		divide(b, num, num);//b=b%num
		if (!mr_compare(b ,one))//b==1, 可能为素数
			continue;
		bool flag = true;
		//测试所有1<=j<k，a^(2^j*m) mod num!=-1
		for (int j = 1; j < k&&flag; j++) {
			if (b == t)
				flag = false;//有一个相等，可能是素数
			else {
				multiply(b, b, b);
				divide(b, num, num);
			}
		}
		if(flag)//如果都不相等，一定是合数
			return false;
	}
	return true;//通过所有测试
}

//随机创建一个素数
void create_prime(int len, big p, int times) {
	bigdig(len, 2, p);
	while (!is_prime(p, times))
		bigdig(len, 2, p);
}


//定义减法
void my_sub(big a, big b, big z) {
	negify(b, b);//b=-b
	add(a, b, z);//z=a+b
	negify(b, b);//b=-b,需要变回来，不会改变参数值
}


/*
//扩展欧几里得算法（递归实现）
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
	copy(a, temp);//暂存a的值
	divide(a, b, a);//a=a/b
	multiply(a, y, a);//a=a*y
	negify(a, a);//a=-a
	add(t, a, y);
	copy(temp, a);
	return r;
}
*/

//扩展欧几里得算法（非递归实现）
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
	copy(a, r);//r暂存a的值
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
		copy(a, r);//暂存a的值
		divide(r, b, b);//r=a%b
		
		my_sub(a, r, a_r);
		copy(a_r, q);
		divide(q, b, q);//q=(a-r)/n
	}
	copy(save_b, b);
	copy(save_a, a);
	return b;
}
