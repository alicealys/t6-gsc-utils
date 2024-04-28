plutonium_sdk = {
    source = path.join(dependencies.basePath, "plutonium-sdk"),
}

function plutonium_sdk.import()
    plutonium_sdk.includes()
end

function plutonium_sdk.includes()
    includedirs {
        plutonium_sdk.source,
    }
end

function plutonium_sdk.project()
end

table.insert(dependencies, plutonium_sdk)
