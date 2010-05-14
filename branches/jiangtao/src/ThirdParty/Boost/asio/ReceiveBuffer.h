// ReceiveBuffer.h file here
#ifndef __ReceiveBuffer_h__
#define __ReceiveBuffer_h__

const long Max_ReceiveBuffer_ReceiveSize	= 5*1024;

//////////////////////////////////////////////
// ReceiveBuffer class
class ReceiveBuffer
{
public:
	typedef boost::shared_ptr<ReceiveBuffer> pointer;
	static pointer createNew(void)
	{
		return pointer(new ReceiveBuffer());
	}

	void reset(void)
	{
		m_size = 0;
		memset(m_buffer, 0, Max_ReceiveBuffer_ReceiveSize);
	}
	const unsigned char * data(void) const {return m_buffer;}

	void size(int newv) {m_size = newv;}
	int size(void) const {return m_size;}

public:
	ReceiveBuffer(void)
	{
		reset();
	}
#ifdef _DEBUG
	~ReceiveBuffer(void)
	{
	}
#endif
private:
    unsigned char m_buffer[Max_ReceiveBuffer_ReceiveSize];
	int m_size;
};

#endif // __ReceiveBuffer_h__
