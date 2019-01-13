#pragma once

int SendEmail_SSL(
	const char *smtp,
	unsigned short port,
	const char *account,
	const char *password,
	const char* email,
	const char* body,
	const char* from_name,
	const char* to_name,
	const char* subject );
