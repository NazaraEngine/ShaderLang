
task("check-files")

set_menu({
	-- Settings menu usage
	usage = "xmake check-files [options]",
	description = "Check every file for consistency (can fix some errors)",
	options =
	{
		-- Set k mode as key-only bool parameter
		{'f', "fix", "k", nil, "Attempt to automatically fix files." }
	}
})

local function CompareLines(referenceLines, lines, firstLine, lineCount)
	firstLine = firstLine or 1
	lineCount = lineCount or (#lines - firstLine + 1)
	if lineCount ~= #referenceLines then
		return false
	end

	for i = 1, lineCount do
		if lines[firstLine + i - 1] ~= referenceLines[i] then
			return false
		end
	end

	return true
end

local function ComputeHeaderFile(filePath)
	local headerPath = filePath:gsub("[\\/]", "/")
	headerPath = headerPath:sub(headerPath:find("NZSL/"), -1)
	headerPath = headerPath:sub(1, headerPath:find("%..+$") - 1) .. ".hpp"

	return headerPath
end

local systemHeaders = {
	["fcntl.h"] = true,
	["mstcpip"] = true,
	["netdb.h"] = true,
	["poll.h"] = true,
	["process.h"] = true,
	["pthread.h"] = true,
	["unistd.h"] = true,
	["windows.h"] = true,
	["winsock2"] = true,
}

local function IsSystemHeader(header)
	if systemHeaders[header] then
		return true
	end

	if header:match("netinet/.*") then
		return true
	end

	if header:match("sys/.*") then
		return true
	end

	return false
end

on_run(function ()
	import("core.base.option")

	local fileLines = {}
	local updatedFiles = {}
	local function GetFile(filePath)
		filePath = path.translate(filePath)

		local lines = fileLines[filePath]
		if not lines then
			lines = table.to_array(io.lines(filePath))
			if not lines then
				os.raise("failed to open " .. filePath)
			end

			fileLines[filePath] = lines
		end

		return lines
	end

	local function UpdateFile(filePath, lines)
		filePath = path.translate(filePath)

		if lines then
			fileLines[filePath] = lines
		end

		updatedFiles[filePath] = true
	end

	local checks = {}

	-- Remove empty lines at the beginning of files
	table.insert(checks, {
		Name = "empty lines",
		Check = function ()
			local files = table.join(
				os.files("include/NZSL/**.hpp"),
				os.files("include/NZSL/**.inl"),
				os.files("src/NZSL/**.hpp"),
				os.files("src/NZSL/**.inl"),
				os.files("src/NZSL/**.cpp")
			)

			local fixes = {}

			for _, filePath in pairs(files) do
				local lines = GetFile(filePath)

				for i = 1, #lines do
					if not lines[i]:match("^%s*$") then
						if i ~= 1 then
							print(filePath .. " starts with empty lines")

							table.insert(fixes, {
								File = filePath,
								Func = function (lines)
									for j = 1, i - 1 do
										table.remove(lines, 1)
									end

									UpdateFile(filePath, lines)
								end
							})
						end
						
						break
					end
				end
			end

			return fixes
		end
	})

	-- Check header guards and #pragma once
	table.insert(checks, {
		Name = "header guards",
		Check = function ()
			local files = table.join(
				os.files("include/NZSL/**.hpp"),
				os.files("src/NZSL/**.hpp")
			)

			local fixes = {}
			for _, filePath in pairs(files) do
				local lines = GetFile(filePath)

				local pragmaLine
				local ifndefLine
				local defineLine
				local endifLine
				local macroName

				local pathMacro = filePath:gsub("[/\\]", "_")
				do
					pathMacro = pathMacro:sub(pathMacro:lastof("NZSL_", true) + 5)
					local i = pathMacro:lastof(".", true)
					if i then
						pathMacro = pathMacro:sub(1, i - 1)
					end
				end

				local pathHeaderGuard = "NZSL_" .. pathMacro:upper() .. "_HPP"

				local canFix = true
				local ignored = false

				-- Fetch pragma once, ifdef and define lines
				for i = 1, #lines do
					if lines[i] == "// no header guards" then
						canFix = false
						ignored = true
						break
					end

					if lines[i] == "#pragma once" then
						if pragmaLine then
							print(filePath .. ": multiple #pragma once found")
							canFix = false
							break
						end

						pragmaLine = i
					elseif not ifndefLine and lines[i]:startswith("#ifndef") then
						ifndefLine = i

						macroName = lines[i]:match("^#ifndef%s+(.+)$")
						if not macroName then
							print(filePath .. ": failed to identify header guard macro (ifndef)")
							canFix = false
							break
						end
					elseif ifndefLine and not defineLine and lines[i]:startswith("#define") then
						defineLine = i

						local defineMacroName = lines[i]:match("^#define%s+(.+)$")
						if not defineMacroName then
							print(filePath .. ": failed to identify header guard macro (define)")
							canFix = false
							break
						end

						if defineMacroName ~= macroName then
							print(filePath .. ": failed to identify header guard macro (define macro doesn't match ifdef)")
							canFix = false
							break
						end
					end

					if ifndefLine and defineLine then
						break
					end
				end

				if not ignored then
					if not ifndefLine or not defineLine or not macroName then
						print(filePath .. ": failed to identify header guard macro")
						canFix = false
					end

					-- Fetch endif line
					if canFix then
						local shouldFixEndif = false

						for i = #lines, 1, -1 do
							if lines[i]:startswith("#endif") then
								local macro = lines[i]:match("#endif // (.+)")
								if macro ~= macroName then
									shouldFixEndif = true
								end

								endifLine = i
								break
							end
						end

						if not endifLine then
							print(filePath .. ": failed to identify header guard macro (endif)")
							canFix = false
						end

						if canFix then
							if macroName ~= pathHeaderGuard then
								print(filePath .. ": header guard mismatch (got " .. macroName .. ", expected " .. pathHeaderGuard .. ")")

								shouldFixEndif = false

								table.insert(fixes, {
									File = filePath,
									Func = function (lines)
										lines[ifndefLine] = "#ifndef " .. pathHeaderGuard
										lines[defineLine] = "#define " .. pathHeaderGuard
										lines[endifLine] = "#endif // " .. pathHeaderGuard

										return lines
									end
								})
							end

							if shouldFixEndif then
								print(filePath .. ": #endif was missing comment")

								table.insert(fixes, {
									File = filePath,
									Func = function (lines)
										lines[endifLine] = "#endif // " .. pathHeaderGuard

										return lines
									end
								})
							end

							if not pragmaLine then
								print(filePath .. ": no #pragma once found")
								table.insert(fixes, {
									File = filePath,
									Func = function (lines)
										table.insert(lines, ifndefLine - 1, "#pragma once")
										table.insert(lines, ifndefLine - 1, "")

										return lines
									end
								})
							elseif pragmaLine > ifndefLine then
								print(filePath .. ": #pragma once is after header guard (should be before)")
							end
						end
					end
				end
			end

			return fixes
		end
	})

	-- Every source file should include its header first
	table.insert(checks, {
		Name = "inclusion",
		Check = function ()
			local files = table.join(
				os.files("include/NZSL/**.inl"),
				os.files("src/NZSL/**.inl"),
				os.files("src/NZSL/**.cpp")
			)

			local fixes = {}
			for _, filePath in pairs(files) do
				local lines = GetFile(filePath)

				local headerPath = ComputeHeaderFile(filePath)
				if os.isfile("include/" .. headerPath) or os.isfile("src/" .. headerPath) then
					local inclusions = {}

					-- Retrieve all inclusions
					for i = 1, #lines do
						if lines[i] == "// no include fix" then
							-- ignore file
							inclusions = {}
							break
						end

						local includeMode, includePath = lines[i]:match("^#include%s*([<\"])(.+)[>\"]")
						if includeMode then
							table.insert(inclusions, {
								line = i,
								mode = includeMode,
								path = includePath
							})
						end
					end

					if #inclusions > 0 then
						local increment = 0

						-- Add corresponding header
						local headerInclude
						for i = 1, #inclusions do
							if inclusions[i].path == headerPath then
								headerInclude = i
								break
							end
						end

						-- Add header inclusion if it's missing
						local isInl = path.extension(filePath) == ".inl"
						if not headerInclude then
							print(filePath .. " is missing corresponding header inclusion")

							table.insert(fixes, {
								File = filePath,
								Func = function (lines)
									local firstHeaderLine = inclusions[1].line
									table.insert(lines, firstHeaderLine, "#include <" .. headerPath .. ">")

									increment = increment + 1

									return lines
								end
							})
						elseif headerInclude and isInl then
							print(filePath .. " has a header inclusion which breaks clangd (.inl should no longer includes their .hpp)")

							table.insert(fixes, {
								File = filePath,
								Func = function (lines)
									table.remove(lines, inclusions[headerInclude].line)
									increment = increment - 1
									return lines
								end
							})
						end
					end
				end
			end

			return fixes
		end
	})

	-- Reorder includes and remove duplicates
	table.insert(checks, {
		Name = "inclusion order",
		Check = function ()
			local files = table.join(
				os.files("include/NZSL/**.hpp"),
				os.files("include/NZSL/**.inl"),
				os.files("src/NZSL/**.hpp"),
				os.files("src/NZSL/**.inl"),
				os.files("src/NZSL/**.cpp")
			)

			local fixes = {}
			for _, filePath in pairs(files) do
				local lines = GetFile(filePath)

				local headerPath = ComputeHeaderFile(filePath)
				
				local inclusions = {}

				-- Retrieve all inclusions from the first inclusion block
				for i = 1, #lines do
					if lines[i] == "// no include reordering" then
						-- ignore file
						inclusions = {}
						break
					end

					local includeMode, includePath = lines[i]:match("^#include%s*([<\"])(.+)[>\"]")
					if includeMode then
						table.insert(inclusions, {
							line = i,
							mode = includeMode,
							path = includePath
						})
					elseif #inclusions > 0 then
						-- Stop when outside the inclusion block
						break
					end
				end

				local includeList = {}
				local shouldReorder = false
				for i = 1, #inclusions do
					local order
					if inclusions[i].path == headerPath then
						order = 0 -- own include comes first
					elseif inclusions[i].path:match("^NazaraUtils/") then
						order = 1.9 -- NazaraUtils
					elseif inclusions[i].path:match("^NZSL/") then
						order = 2 -- project includes
					elseif IsSystemHeader(inclusions[i].path) then
						order = 5 -- system includes
					elseif inclusions[i].path:match(".+%.hp?p?") then
						order = 3 -- thirdparty includes
					else
						order = 4 -- standard includes
					end

					table.insert(includeList, {
						order = order,
						path = inclusions[i].path,
						content = lines[inclusions[i].line]
					})
				end

				local function compareFunc(a, b)
					if a.order == b.order then
						local _, folderCountA = a.path:gsub("/", "")
						local _, folderCountB = b.path:gsub("/", "")
						if folderCountA == folderCountB then
							return a.path < b.path
						else
							return folderCountA < folderCountB 
						end
					else
						return a.order < b.order
					end
				end

				local isOrdered = true
				for i = 2, #includeList do
					if includeList[i - 1].path == includeList[i].path then
						-- duplicate found
						print(filePath .. ": include list has duplicates")
						isOrdered = false
						break
					end

					if not compareFunc(includeList[i - 1], includeList[i]) then
						print(filePath .. ": include list is not ordered")
						isOrdered = false
						break
					end
				end

				if not isOrdered then
					table.sort(includeList, compareFunc)

					table.insert(fixes, {
						File = filePath,
						Func = function (lines)
							-- Reorder includes
							local newInclusions = {}
							for i = 1, #inclusions do
								lines[inclusions[i].line] = includeList[i].content
								table.insert(newInclusions, {
									content = includeList[i].content,
									path = includeList[i].path,
									line = inclusions[i].line
								})
							end

							-- Remove duplicates
							table.sort(newInclusions, function (a, b) return a.line > b.line end)

							for i = 2, #newInclusions do
								local a = newInclusions[i - 1]
								local b = newInclusions[i]

								if a.path == b.path then
									if #a.content > #b.content then -- keep longest line (for comments)
										table.remove(lines, b.line)
									else
										table.remove(lines, a.line)
									end
								end
							end

							return lines
						end
					})
				end
			end

			return fixes
		end
	})

	-- Check copyright date and format
	table.insert(checks, {
		Name = "copyright",
		Check = function ()
			local files = table.join(
				os.files("include/NZSL/**.hpp|Config.hpp"),
				os.files("include/NZSL/**.inl"),
				os.files("src/NZSL/**.hpp"),
				os.files("src/NZSL/**.inl"),
				os.files("src/NZSL/**.cpp")
			)

			local currentYear = os.date("%Y")
			local engineAuthor = "Jérôme \"Lynix\" Leclercq (lynix680@gmail.com)"
			local prevAuthor = "Jérôme Leclercq"
			local fixes = {}

			-- Config
			do
				local configFilePath = path.translate("include/NZSL/Config.hpp")
				local lines = GetFile(configFilePath)
				local pragmaLine
				for i = 1, #lines do
					if lines[i] == "#pragma once" then
						pragmaLine = i
						break
					end
				end

				local canFix = true

				repeat
					if not pragmaLine then
						print(configFilePath .. ": pragma once not found")
						canFix = false
						break
					end

					local licenseText = [[

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
	of the Software, and to permit persons to whom the Software is furnished to do
	so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once]]

					local licenseLines = licenseText:split("\r?\n", { strict = true })
					local shouldFix = false

					-- Try to retrieve year and authors
					if lines[1] ~= "/*" then
						print(configFilePath .. ": file doesn't begin with block comment")
						break
					end


					if lines[2]:match("\tNazara Shading Language (NZSL)") then
						print(configFilePath .. ": module description doesn't match")
						shouldFix = true
					end

					if lines[3] ~= "" then
						-- Not really empty because of space characters? Not a big deal
						if lines[3]:match("^%s*$") then
							shouldFix = true
						else
							print(configFilePath .. ": expected space after project name")
							break
						end
					end

					local year, moduleAuthor = lines[4]:match("^\tCopyright %(C%) (Y?E?A?R?%d*) (.+)$")
					if not year then
						print(configFilePath .. ": couldn't parse copyright date and author")
						break
					end

					if year ~= currentYear then
						print(configFilePath .. ": incorrect copyright year")
						shouldFix = true
					end

					local additionalAuthors = {}
					for i = 5, #lines do
						if lines[i]:match("^%s*$") then
							-- Empty line
							if not CompareLines(licenseLines, lines, i, pragmaLine - i + 1) then
								shouldFix = true
							end

							break
						end

						if lines[i]:match("%s*Permission is hereby granted") then
							print(configFilePath .. ": missing empty line before licence text")
							shouldFix = true
							break
						end

						table.insert(additionalAuthors, lines[i]:match("%s*(.+)"))
					end

					if shouldFix then
						table.insert(fixes, {
							File = configFilePath,
							Func = function (lines)
								local newLines = {
									"/*",
									"\tNazara Shading Language (NZSL)",
									"",
								}

								local copyrightNotice = "Copyright (C) " .. currentYear
								table.insert(newLines, "\t" .. copyrightNotice .. " " .. moduleAuthor)

								for _, author in ipairs(additionalAuthors) do
									table.insert(newLines, "\t" .. string.rep(" ", #copyrightNotice) .. " " .. author)
								end

								for _, line in ipairs(licenseLines) do
									table.insert(newLines, line)
								end

								for i = pragmaLine + 1, #lines do
									table.insert(newLines, lines[i])
								end

								return newLines
							end
						})
					end
				until true

				if not canFix then
					print(configFilePath .. ": header couldn't be parsed, no automated fix will be done")
				end
			end

			-- Headers
			for _, filePath in pairs(files) do
				local lines = GetFile(filePath)

				local hasCopyright
				local shouldFix = false

				if lines[1]:lower():match("^// this file was automatically generated") then
					goto skip
				end

				local year, authors = lines[1]:match("^// Copyright %(C%) (Y?E?A?R?%d*) (.+)$")
				hasCopyright = year ~= nil

				if not authors or authors == "AUTHORS" then
					authors = engineAuthor
				else
					local fixedAuthors = authors:gsub(prevAuthor, engineAuthor)
					if fixedAuthors ~= authors then
						authors = fixedAuthors
						shouldFix = true
					end
				end

				if hasCopyright then
					if year ~= currentYear then
						print(filePath .. ": copyright year error")
						shouldFix = true
					end

					if lines[2] ~= [[// This file is part of the "Nazara Shading Language" project]] then
						print(filePath .. ": copyright module error")
						shouldFix = true
					end

					if lines[3] ~= "// For conditions of distribution and use, see copyright notice in Config.hpp" then
						print(filePath .. ": copyright file reference error")
						shouldFix = true
					end
				else
					print(filePath .. ": copyright not found")
					shouldFix = true
				end

				if shouldFix then
					table.insert(fixes, {
						File = filePath,
						Func = function(lines)
							local copyrightLines = {
								"// Copyright (C) " .. currentYear .. " " .. (authors or engineAuthor),
								[[// This file is part of the "Nazara Shading Language" project]],
								"// For conditions of distribution and use, see copyright notice in Config.hpp"
							}

							if hasCopyright then
								for i, line in ipairs(copyrightLines) do
									lines[i] = line
								end
							else
								for i, line in ipairs(copyrightLines) do
									table.insert(lines, i, line)
								end
								table.insert(lines, #copyrightLines + 1, "")
							end

							return lines
						end
					})
				end

				::skip::
			end

			return fixes
		end
	})

	local shouldFix = option.get("fix") or false

	for _, check in pairs(checks) do
		print("Running " .. check.Name .. " check...")

		local fixes = check.Check(moduleName)

		if shouldFix then
			for _, fix in pairs(fixes) do
				print("Fixing " .. fix.File)
				UpdateFile(fix.File, fix.Func(assert(fileLines[fix.File])))
			end
		end
	end

	for filePath, _ in pairs(updatedFiles) do
		local lines = assert(fileLines[filePath])
		if lines[#lines] ~= "" then
			table.insert(lines, "")
		end

		print("Saving changes to " .. filePath)
		io.writefile(filePath, table.concat(lines, "\n"))
	end
end)
