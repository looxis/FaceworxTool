#pragma once

template <char off,char len>
struct calc_mask
{
	enum {val=((1<<len)-1)<<off};
};

template <
	char a_off,char a_len,
	char r_off,char r_len,
	char g_off,char g_len,
	char b_off,char b_len,
	char bytes,typename type
>
struct pixeler
{
	enum {a_off_bits=a_off%8};
	enum {a_off_bytes=a_off/8};
	enum {a_mask=calc_mask<a_off,a_len>::val};
	enum {a_mask_type=calc_mask<a_off_bits,a_len>::val};
	enum {a_maxval=(1<<a_len)-1};
	enum {a_simple=a_off_bits==0 && sizeof(type)*8==a_len};

	enum {r_off_bits=r_off%8};
	enum {r_off_bytes=r_off/8};
	enum {r_mask=calc_mask<r_off,r_len>::val};
	enum {r_mask_type=calc_mask<r_off_bits,r_len>::val};
	enum {r_maxval=(1<<r_len)-1};
	enum {r_simple=r_off_bits==0 && sizeof(type)*8==r_len};

	enum {g_off_bits=g_off%8};
	enum {g_off_bytes=g_off/8};
	enum {g_mask=calc_mask<g_off,g_len>::val};
	enum {g_mask_type=calc_mask<g_off_bits,g_len>::val};
	enum {g_maxval=(1<<g_len)-1};
	enum {g_simple=g_off_bits==0 && sizeof(type)*8==g_len};

	enum {b_off_bits=b_off%8};
	enum {b_off_bytes=b_off/8};
	enum {b_mask=calc_mask<b_off,b_len>::val};
	enum {b_mask_type=calc_mask<b_off_bits,b_len>::val};
	enum {b_maxval=(1<<b_len)-1};
	enum {b_simple=b_off_bits==0 && sizeof(type)*8==b_len};

	template <int qwe> static void a_set(type& q1,type val);
	template <> static void a_set<1>(type& q1,type val)
	{
		q1=val;
	}
	template <> static void a_set<0>(type& q1,type val)
	{
		q1&=~(type)a_mask_type;
		q1|=val<<a_off_bits;
	}
//
	template <int qwe> static void r_set(type& q1,type val);
	template <> static void r_set<1>(type& q1,type val)
	{
		q1=val;
	}
	template <> static void r_set<0>(type& q1,type val)
	{
		q1&=~(type)r_mask_type;
		q1|=val<<r_off_bits;
	}
//
	template <int qwe> static void g_set(type& q1,type val);
	template <> static void g_set<1>(type& q1,type val)
	{
		q1=val;
	}
	template <> static void g_set<0>(type& q1,type val)
	{
		q1&=~(type)g_mask_type;
		q1|=val<<g_off_bits;
	}
//
	template <int qwe> static void b_set(type& q1,type val);
	template <> static void b_set<1>(type& q1,type val)
	{
		q1=val;
	}
	template <> static void b_set<0>(type& q1,type val)
	{
		q1&=~(type)b_mask_type;
		q1|=val<<b_off_bits;
	}

	template <int len,bool b> struct modify_helper{};
	template <int len> struct modify_helper<len,true>
	{
		static __forceinline type modify_val(unsigned char val)
		{
			return val>>(8-len);
		}
	};
	template <int len> struct modify_helper<len,false>
	{
		static __forceinline type modify_val(unsigned char val)
		{
			return val<<(len-8);
		}
	};

	template <int len> static __forceinline type modify_val(unsigned char v)
	{
		return modify_helper<len,len<=8>::modify_val(v);
	}

	static __forceinline void iterate_line(char*& p,unsigned char r,unsigned char g,unsigned char b,unsigned char a)
	{
		r_set<r_simple>(*(type*)(p+r_off_bytes),modify_val<r_len>(r));
		g_set<g_simple>(*(type*)(p+g_off_bytes),modify_val<g_len>(g));
		b_set<b_simple>(*(type*)(p+b_off_bytes),modify_val<b_len>(b));
		a_set<a_simple>(*(type*)(p+a_off_bytes),modify_val<a_len>(a));
		p+=bytes;
	}
};