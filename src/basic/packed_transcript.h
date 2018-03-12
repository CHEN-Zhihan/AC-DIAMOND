/****
Copyright (c) 2015, University of Tuebingen
Author: Benjamin Buchfink
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****/

#ifndef PACKED_TRANSCRIPT_H_
#define PACKED_TRANSCRIPT_H_

typedef enum { op_match=0, op_insertion=1, op_deletion=2, op_substitution=3 } Edit_operation;

struct Packed_operation
{
	Packed_operation(uint8_t code):
		code (code)
	{ }
	Packed_operation(Edit_operation op, unsigned count):
		code ((op<<6) | count)
	{ }
	template<typename _val>
	Packed_operation(Edit_operation op, _val v):
		code ((op<<6) | (int)v)
	{ }
	operator uint8_t() const
	{ return code; }
	Edit_operation op() const
	{ return (Edit_operation)(code>>6); }
	unsigned count() const
	{ return code&63; }
	template<typename _val>
	_val letter() const
	{ return code&63; }
	static Packed_operation terminator()
	{ return Packed_operation(op_match, 0); }
	uint8_t code;
};

template<typename _val>
struct Combined_operation
{
	Edit_operation op;
	unsigned count;
	_val letter;
};

struct Packed_transcript
{

	template<typename _val>
	struct Const_iterator
	{
		Const_iterator(const Packed_operation *op):
			ptr_ (op)
		{ gather(); }
		bool good() const
		{ return *ptr_ != Packed_operation::terminator(); }
		Const_iterator& operator++()
		{ ++ptr_; gather(); return *this; }
		const Combined_operation<_val>& operator*() const
		{ return op_; }
		const Combined_operation<_val>* operator->() const
		{ return &op_; }
	private:
		void gather()
		{
			if(!good())
				return;
			op_.op = ptr_->op();
			if(op_.op == op_deletion || op_.op == op_substitution) {
				op_.letter = ptr_->letter<_val>();
				op_.count = 1;
			} else {
				op_.count = 0;
				do {
					op_.count += ptr_->count();
					++ptr_;
				} while(good() && ptr_->op() == op_.op);
				--ptr_;
			}
		}
		const Packed_operation *ptr_;
		Combined_operation<_val> op_;
	};

	void read(Buffered_file &f)
	{
		data_.clear();
		uint8_t code;
		do {
			f.read(code);
			data_.push_back(code);
		} while (code != Packed_operation::terminator());
	}

	void read(Binary_buffer::Iterator &it)
	{
		data_.clear();
		uint8_t code;
		do {
			it >> code;
			data_.push_back(code);
		} while (code != Packed_operation::terminator());
	}

	template<typename _val>
	Const_iterator<_val> begin() const
	{ return Const_iterator<_val> (data_.data()); }

	const vector<Packed_operation>& data() const
	{ return data_; }

private:

	vector<Packed_operation> data_;

};

#endif /* PACKED_TRANSCRIPT_H_ */
