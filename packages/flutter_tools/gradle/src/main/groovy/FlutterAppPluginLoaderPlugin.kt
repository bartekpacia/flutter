import groovy.json.JsonSlurper
import org.gradle.api.Plugin
import org.gradle.api.initialization.Settings
import java.io.File

//@Serializable
//data class FlutterPluginsDependencies(
//    val info: String,
//    val plugins: Map<String, List<PluginEntry>>,
//)
//
//@Serializable
//data class PluginEntry(
//    val name: String,
//    val path: String,
//    @SerialName("native_build")
//    val nativeBuild: Boolean,
//)

/**
 * Format of the .flutter-plugins-dependencies file.
 */
typealias FlutterPluginsDependencies = Map<String, Map<String, List<Map<String, String>>>>

class FlutterAppPluginLoaderPluginKTS : Plugin<Settings> {
    // This string must match _kFlutterPluginsHasNativeBuildKey defined in
    // packages/flutter_tools/lib/src/flutter_plugins.dart.
    private val nativeBuildKey: String = "native_build"

    override fun apply(settings: Settings) {
        val flutterProjectRoot: File = settings.settingsDir.parentFile

        // If this logic is changed, also change the logic in module_plugin_loader.gradle.
        val pluginsFile: File = File(flutterProjectRoot, ".flutter-plugins-dependencies")
        if (!pluginsFile.exists()) {
            return
        }

        val obj = JsonSlurper().parseText(pluginsFile.readText()) as FlutterPluginsDependencies

        obj["plugins"]!!["android"]!!.forEach { androidPlugin ->
            val pluginName = androidPlugin["name"] as String
            val pluginPath = androidPlugin["path"] as String

            val needsBuild =
                if (androidPlugin.containsKey(nativeBuildKey)) (!androidPlugin[nativeBuildKey].toBoolean()) else true
            if (!needsBuild) {
                println("$pluginName does not need build")
                return
            } else {
                println("$pluginName needs build")
            }

            val pluginDirectory = File(pluginPath, "android")
            assert(pluginDirectory.exists()) { "Plugin directory $pluginDirectory does not exist." }

            val pluginProjectName = ":$pluginName"

            settings.include(pluginProjectName)
            settings.project(pluginProjectName).projectDir = pluginDirectory
        }
    }

}
