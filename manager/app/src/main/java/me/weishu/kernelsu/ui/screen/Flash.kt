package me.weishu.kernelsu.ui.screen

import android.net.Uri
import android.os.Environment
import android.os.Parcelable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Save
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.key.Key
import androidx.compose.ui.input.key.key
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.ramcosta.composedestinations.annotation.Destination
import com.ramcosta.composedestinations.navigation.DestinationsNavigator
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlinx.parcelize.Parcelize
import me.weishu.kernelsu.R
import me.weishu.kernelsu.ui.component.KeyEventBlocker
import me.weishu.kernelsu.ui.util.LocalSnackbarHost
import me.weishu.kernelsu.ui.util.installBoot
import me.weishu.kernelsu.ui.util.installModule
import me.weishu.kernelsu.ui.util.reboot
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

/**
 * @author weishu
 * @date 2023/1/1.
 */
@OptIn(ExperimentalComposeUiApi::class)
@Composable
@Destination
fun FlashScreen(navigator: DestinationsNavigator, flashIt: FlashIt) {

    var text by rememberSaveable { mutableStateOf("") }
    val logContent = rememberSaveable { StringBuilder() }
    var showFloatAction by rememberSaveable { mutableStateOf(false) }

    val snackBarHost = LocalSnackbarHost.current
    val scope = rememberCoroutineScope()
    val scrollState = rememberScrollState()

    LaunchedEffect(Unit) {
        if (text.isNotEmpty()) {
            return@LaunchedEffect
        }
        withContext(Dispatchers.IO) {
            flashIt(flashIt, onFinish = { showReboot ->
                if (showReboot) {
                    showFloatAction = true
                }
            }, onStdout = {
                text += "$it\n"
                logContent.append(it).append("\n")
            }, onStderr = {
                logContent.append(it).append("\n")
            });
        }
    }

    Scaffold(
        topBar = {
            TopBar(
                onBack = {
                    navigator.popBackStack()
                },
                onSave = {
                    scope.launch {
                        val format = SimpleDateFormat("yyyy-MM-dd-HH-mm-ss", Locale.getDefault())
                        val date = format.format(Date())
                        val file = File(
                            Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),
                            "KernelSU_install_log_${date}.log"
                        )
                        file.writeText(logContent.toString())
                        snackBarHost.showSnackbar("Log saved to ${file.absolutePath}")
                    }
                }
            )
        },
        floatingActionButton = {
            if (showFloatAction) {
                val reboot = stringResource(id = R.string.reboot)
                ExtendedFloatingActionButton(
                    onClick = {
                        scope.launch {
                            withContext(Dispatchers.IO) {
                                reboot()
                            }
                        }
                    },
                    icon = { Icon(Icons.Filled.Refresh, reboot) },
                    text = { Text(text = reboot) },
                )
            }

        }
    ) { innerPadding ->
        KeyEventBlocker {
            it.key == Key.VolumeDown || it.key == Key.VolumeUp
        }
        Column(
            modifier = Modifier
                .fillMaxSize(1f)
                .padding(innerPadding)
                .verticalScroll(scrollState),
        ) {
            LaunchedEffect(text) {
                scrollState.animateScrollTo(scrollState.maxValue)
            }
            Text(
                modifier = Modifier.padding(8.dp),
                text = text,
                fontSize = MaterialTheme.typography.bodySmall.fontSize,
                fontFamily = FontFamily.Monospace,
                lineHeight = MaterialTheme.typography.bodySmall.lineHeight,
            )
        }
    }
}

@Parcelize
sealed class FlashIt : Parcelable {
    data class FlashBoot(val bootUri: Uri? = null, val koUri: Uri, val ota: Boolean) : FlashIt()

    data class FlashModule(val uri: Uri) : FlashIt()
}

fun flashIt(
    flashIt: FlashIt, onFinish: (Boolean) -> Unit,
    onStdout: (String) -> Unit,
    onStderr: (String) -> Unit
) {
    when (flashIt) {
        is FlashIt.FlashBoot -> installBoot(
            flashIt.bootUri,
            flashIt.koUri,
            flashIt.ota,
            onFinish,
            onStdout,
            onStderr
        )

        is FlashIt.FlashModule -> installModule(flashIt.uri, onFinish, onStdout, onStderr)
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun TopBar(onBack: () -> Unit = {}, onSave: () -> Unit = {}) {
    TopAppBar(
        title = { Text(stringResource(R.string.install)) },
        navigationIcon = {
            IconButton(
                onClick = onBack
            ) { Icon(Icons.Filled.ArrowBack, contentDescription = null) }
        },
        actions = {
            IconButton(onClick = onSave) {
                Icon(
                    imageVector = Icons.Filled.Save,
                    contentDescription = "Localized description"
                )
            }
        }
    )
}

@Preview
@Composable
fun InstallPreview() {
//    InstallScreen(DestinationsNavigator(), uri = Uri.EMPTY)
}