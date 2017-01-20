int get_int64_value_from_ascii_string (std::string source_string, int char_index, int64_t *result)
{
	int32_t value = -1;
	int32_t value_last = 0;
	char *p_source_string;
	
	
	p_source_string = (char*)source_string.c_str();
	p_source_string += char_index;

	//Ignore any leading spaces
	while (*p_source_string == ' ')
		p_source_string++;

	while ((*p_source_string >= '0') && (*p_source_string <= '9'))
	{
		if (value < 0)
			value = 0;
		value_last = value;

		value *= 10;
		value += (*p_source_string++ - 0x30);

		if (value_last > value)
		{
			value = -1;				//Value is > max possible
			break;
		}
	}
	*result = value;
	return((int)(p_source_string - (char*)source_string.c_str()));
}

std::string do_console_command_get_result (char* command)
{
	FILE* pipe = popen(command, "r");
	if (!pipe)
		return "ERROR";
	
	char buffer[128];
	std::string result = "";
	while(!feof(pipe))
	{
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return(result);
}