
#include <Windows.h>

/* IOCTL e o read*/

// Solicita��o para ler mem�ria de usu�rio virtual (mem�ria de um programa) do espa�o do kernel
#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701 /*  */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Solicita��o para escrever mem�ria de usu�rio virtual (mem�ria de um programa) a partir do espa�o do kernel
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0702 /* */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Solicita��o para recuperar o processo de id do csgo, do espa�o kernel
#define IO_GET_ID_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0703 /* */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Solicita��o para recuperar o endere�o base do cliente.dll no csgo.exe do espa�o do kernel
#define IO_GET_MODULE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0704 /*  */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Offset for�ar pulo
#define FORCE_JUMP 0x04F5FD5C
// offset player local
#define LOCAL_PLAYER 0x00AA66D4

#define FFLAGS 0x00000100


typedef struct _KERNEL_READ_REQUEST
{
	ULONG ProcessId;

	ULONG Address;
	ULONG Response;
	ULONG Size;

} KERNEL_READ_REQUEST, *PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST
{
	ULONG ProcessId;

	ULONG Address;
	ULONG Value;
	ULONG Size;

} KERNEL_WRITE_REQUEST, *PKERNEL_WRITE_REQUEST;



class KeInterface
{
public:
	HANDLE hDriver; 

					// iniciar
	KeInterface::KeInterface(LPCSTR RegistryPath)
	{
		hDriver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	}

	template <typename type>
	type ReadVirtualMemory(ULONG ProcessId, ULONG ReadAddress,
		SIZE_T Size)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return (type)false;

		DWORD Return, Bytes;
		KERNEL_READ_REQUEST ReadRequest;

		ReadRequest.ProcessId = ProcessId;
		ReadRequest.Address = ReadAddress;
		ReadRequest.Size = Size;

		// mandar codigo pro driver c argumentos
		if (DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadRequest,
			sizeof(ReadRequest), &ReadRequest, sizeof(ReadRequest), 0, 0))
			return (type)ReadRequest.Response;
		else
			return (type)false;
	}

	bool WriteVirtualMemory(ULONG ProcessId, ULONG WriteAddress,
		ULONG WriteValue, SIZE_T WriteSize)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;
		DWORD Bytes;

		KERNEL_WRITE_REQUEST  WriteRequest;
		WriteRequest.ProcessId = ProcessId;
		WriteRequest.Address = WriteAddress;
		WriteRequest.Value = WriteValue;
		WriteRequest.Size = WriteSize;

		if (DeviceIoControl(hDriver, IO_WRITE_REQUEST, &WriteRequest, sizeof(WriteRequest),
			0, 0, &Bytes, NULL))
			return true;
		else
			return false;
	}

	DWORD GetTargetPid()
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;

		ULONG Id;
		DWORD Bytes;

		if (DeviceIoControl(hDriver, IO_GET_ID_REQUEST, &Id, sizeof(Id),
			&Id, sizeof(Id), &Bytes, NULL))
			return Id;
		else
			return false;
	}

	DWORD GetClientModule()
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;

		ULONG Address;
		DWORD Bytes;

		if (DeviceIoControl(hDriver, IO_GET_MODULE_REQUEST, &Address, sizeof(Address),
			&Address, sizeof(Address), &Bytes, NULL))
			return Address;
		else
			return false;
	}
};
