#include <stdio.h> // size_t

/*
**	This interface class provides a model
**	for all classes destined to handle
**	the packet that will be send in response
**	to a http-request
*/

class IresponsePacket {

	public:
		IresponsePacket() {};
		~IresponsePacket() {};
		virtual void	setData() = 0;
		virtual char	*getData() = 0;
		virtual size_t	getDataSize() = 0;
		virtual void	setDataSize() = 0;

	protected:
		char*	_data;
		size_t	_dataSize;

}; // IresponsePacket
