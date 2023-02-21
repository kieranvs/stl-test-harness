#pragma once

enum class TestResult
{
	IncorrectResults,
	LeaksMemory,
	IncorrectObjectHandling,
	SuboptimalObjectHandling,
	Pass
};

void output_result(const char* name, TestResult result)
{
	if (result == TestResult::Pass)
		printf("  %s: \033[32mpass\033[0m\n", name);
	else if (result == TestResult::SuboptimalObjectHandling)
		printf("  %s: \033[32mpass, suboptimal copies/moves\033[0m\n", name);
	else if (result == TestResult::IncorrectObjectHandling)
		printf("  %s: \033[33mincorrect object handling\033[0m\n", name);
	else if (result == TestResult::LeaksMemory)
		printf("  %s: \033[33mleaks memory\033[0m\n", name);
	else if (result == TestResult::IncorrectResults)
		printf("  %s: \033[31mfail\033[0m\n", name);
}

void output_warning(const char* name, const char* warning)
{
	printf("  %s: \033[33m%s\033[0m\n", name, warning);
}