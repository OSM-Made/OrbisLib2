#include "../External/libjbc/jailbreak.h"
#include "LibJbc.h"

bool Jailbroken = false;
jbc_cred cred;

bool Jailbreak()
{
	if (!Jailbroken)
	{
		if (!jbc_get_cred(&cred)) return false;

		jbc_cred jbcred = cred;
		if (!jbc_jailbreak_cred(&jbcred)) return false;

		if (!jbc_set_cred(&jbcred)) return false;

		Jailbroken = true;
		return true;
	}
	
	return true;
}

bool RestoreJail()
{
	if (Jailbroken)
	{
		if (!jbc_set_cred(&cred)) return false;

		Jailbroken = false;
		return true;
	}

	return true;
}