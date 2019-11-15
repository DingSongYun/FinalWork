#include "EditorLuaFileHelper.h"
#include "FileHelper.h"
#include "Regex.h"

FString FEditorLuaFileHelper::ReadValueFromLuaTable(const FString& tKey, const TCHAR* tFileName)
{
	const FString fileAbsPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + tFileName);
	TArray<FString> contentList;
	FRegexPattern pattern(TEXT("^[a-zA-Z0-9_]+ "));
	FFileHelper::LoadFileToStringArray(contentList, *fileAbsPath);
	for (int32 i = 0; i < contentList.Num(); i++)
	{
		if (contentList[i].Contains(tKey))
		{
			FRegexMatcher matcher(pattern, contentList[i].TrimStart());
			if (matcher.FindNext())
			{
				FString* ls = nullptr;
				FString* rs = nullptr;
				contentList[i].TrimStart().Split(FString("="), ls, rs);
				return (*rs).TrimStart();
			}
		}
	}
	return FString("");
}

void FEditorLuaFileHelper::SaveValueToLuaTable(const FString& tKey, const FString& tValue, const TCHAR* tFileName)
{
	const FString fileAbsPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + tFileName);
	TArray<FString> contentList;
	TArray<FString> outputList;
	FFileHelper::LoadFileToStringArray(contentList, *fileAbsPath);
	FRegexPattern pattern(TEXT("^[a-zA-Z0-9_]+"));
	bool isMatch = false;
	for (int32 i = 0; i < contentList.Num() - 1; i++)
	{
		if (true == isMatch)
		{
			outputList.Add(contentList[i]);
			continue;
		}
		if (contentList[i].Contains(tKey))
		{
			FRegexMatcher matcher(pattern, contentList[i].TrimStart());
			if (matcher.FindNext())
			{
				// Make sure it's an exact match.
				FString temp(matcher.GetCaptureGroup(0));
				if (temp == tKey)
				{
					outputList.Add(tKey + TEXT(" = ") + tValue);
					isMatch = true;
					continue;
				}
				else
				{
					outputList.Add(contentList[i]);
				}
			}
		}
		outputList.Add(contentList[i]);
	}
	// If not found, add to last line.
	if (!isMatch)
	{
		outputList.Add(tKey + TEXT(" = ") + tValue);
	}
	else
	{
		outputList.Add(contentList[contentList.Num() - 1]);
	}
	FFileHelper::SaveStringArrayToFile(outputList, *fileAbsPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}
