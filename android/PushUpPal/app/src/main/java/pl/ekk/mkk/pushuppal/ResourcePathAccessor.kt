package pl.ekk.mkk.pushuppal

import android.content.Context

import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream

/**
 * Created by kowalczm on 08/10/2017.
 */

object ResourcePathAccessor {
    private val CLASSIFIER_FILE = "haarcascade_frontalface_alt2-mkk1.xml"

    @Throws(IOException::class)
    private fun copyRawToPath(context: Context, id: Int, path: String) {
        val inputStream = context.resources.openRawResource(id)
        val outputStream = FileOutputStream(path)
        inputStream.use { input ->
            outputStream.use { fileOut ->
                input.copyTo(fileOut)
            }
        }
    }

    @Throws(IOException::class)
    private fun copyRawToFileIfNotExist(context: Context, id: Int, file: File) {
        if (!file.exists()) {
            copyRawToPath(context, id, file.absolutePath)
        }
    }

    @Throws(IOException::class)
    fun getClassifierFilePath(context: Context): String {
        val internalClassifierFile = File(context.filesDir, CLASSIFIER_FILE)
        copyRawToFileIfNotExist(context, R.raw.haarcascade_frontalface_alt2, internalClassifierFile)
        return internalClassifierFile.absolutePath
    }
}
