sqlpp11 = {
	source = path.join(dependencies.basePath, "sqlpp11"),
}

function sqlpp11.import()
	sqlpp11.includes()
end

function sqlpp11.includes()
	includedirs {
		path.join(sqlpp11.source, "include"),
		path.join(dependencies.basePath, "date/include"),
	}
end

function sqlpp11.project()
	project "sqlpp11"
		language "C++"

		sqlpp11.includes()

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, sqlpp11)
