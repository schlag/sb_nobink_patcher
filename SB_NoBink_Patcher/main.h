class CCRC
{
protected:
	unsigned long crc32_table[256];  // Lookup table array
	unsigned long Reflect(unsigned long ref, char ch);  // Reflects CRC bits in the lookup table
public:
	void Init_CRC32_Table();  // Builds lookup table array
	int Get_CRC(char* data, int size);  // Creates a CRC from a text string 
};

long GetShortcutTarget(const char* lpszLinkFile, char* lpszPath);
void StripFilename(char *path);