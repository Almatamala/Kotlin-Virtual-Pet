package com.example.kotlin_virtualpet

import android.animation.ObjectAnimator
import android.animation.ValueAnimator
import android.app.AlertDialog
import android.app.Dialog
import android.graphics.Color
import android.graphics.Typeface
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.animation.LinearInterpolator
import android.widget.*
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.content.res.ResourcesCompat
import androidx.core.graphics.toColorInt
import androidx.core.graphics.drawable.toDrawable
import com.google.androidgamesdk.GameActivity
import com.skydoves.colorpickerview.ColorPickerDialog
import com.skydoves.colorpickerview.listeners.ColorEnvelopeListener

class MainActivity : GameActivity() {

    private var currentColorInt: Int = Color.WHITE
    private var loadingOverlay: FrameLayout? = null
    private var isRendererReady = false

    companion object {
        init {
            System.loadLibrary("kotlin_virtualpet")
        }
    }

    external fun setVHSEffectNative(enabled: Boolean)
    external fun isVHSEnabledNative(): Boolean
    external fun setPetColorNative(r: Float, g: Float, b: Float)

    override fun onCreate(savedInstanceState: Bundle?) {
        AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES)
        super.onCreate(savedInstanceState)

        // Mostrar pantalla de carga al inicio
        showLoadingScreen()

        // El renderer se inicializará en segundo plano
        // Ocultar la pantalla de carga después de que esté listo
        Handler(Looper.getMainLooper()).postDelayed({
            hideLoadingScreen()
            isRendererReady = true
        }, 500) // Dar tiempo para la primera inicialización

        val btnSettings = ImageButton(this).apply {
            setImageResource(R.drawable.settings_icon)
            setBackgroundColor(Color.TRANSPARENT)
            alpha = 0f // Inicialmente invisible
        }

        val params = FrameLayout.LayoutParams(180, 180).apply {
            gravity = Gravity.BOTTOM or Gravity.END
            setMargins(0, 0, 50, 50)
        }

        addContentView(btnSettings, params)

        // Animar entrada del botón después de que cargue
        Handler(Looper.getMainLooper()).postDelayed({
            btnSettings.animate()
                .alpha(1f)
                .setDuration(300)
                .start()
        }, 600)

        btnSettings.setOnClickListener { showConfigurationScreen() }
    }

    override fun onResume() {
        super.onResume()

        // Mostrar pantalla de carga al reanudar (si el renderer no está listo)
        if (!isRendererReady) {
            showLoadingScreen()

            // Dar tiempo para que el contexto GL se estabilice
            Handler(Looper.getMainLooper()).postDelayed({
                hideLoadingScreen()
                isRendererReady = true
            }, 250) // Delay más corto al reanudar
        }
    }

    override fun onPause() {
        super.onPause()
        isRendererReady = false
    }

    private fun showLoadingScreen() {
        if (loadingOverlay != null) return // Ya existe

        val overlay = FrameLayout(this).apply {
            setBackgroundColor("#121212".toColorInt())
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        }

        // Contenedor central
        val contentLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
        }

        // Texto "Cargando..."
        val loadingText = TextView(this).apply {
            text = "Cargando..."
            textSize = 24f
            setTextColor(Color.WHITE)
            typeface = try {
                ResourcesCompat.getFont(context, R.font.bitcountsingle)
            } catch (e: Exception) {
                Typeface.MONOSPACE
            }
        }

        // Barra de progreso con animación
        val progressBar = ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal).apply {
            isIndeterminate = false
            max = 100
            progress = 0
            layoutParams = LinearLayout.LayoutParams(400, 20).apply {
                topMargin = 40
            }
        }

        // Animar la barra de progreso
        val animator = ObjectAnimator.ofInt(progressBar, "progress", 0, 100).apply {
            duration = 400 // Duración de la animación
            interpolator = LinearInterpolator()
            start()
        }

        contentLayout.addView(loadingText)
        contentLayout.addView(progressBar)

        val centerParams = FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        ).apply {
            gravity = Gravity.CENTER
        }

        overlay.addView(contentLayout, centerParams)

        loadingOverlay = overlay
        addContentView(overlay, FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        ))
    }

    private fun hideLoadingScreen() {
        loadingOverlay?.let { overlay ->
            // Animar fade out
            overlay.animate()
                .alpha(0f)
                .setDuration(300)
                .withEndAction {
                    (overlay.parent as? ViewGroup)?.removeView(overlay)
                    loadingOverlay = null
                }
                .start()
        }
    }

    private fun showConfigurationScreen() {
        val bitcount = try {
            ResourcesCompat.getFont(this, R.font.bitcountsingle)
        } catch (e: Exception) {
            Typeface.MONOSPACE
        }

        val configDialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)

        val rootLayout = FrameLayout(this).apply {
            setBackgroundColor("#121212".toColorInt())
        }

        val controlsLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setPadding(50, 50, 50, 50)
        }

        val titleMenu = TextView(this).apply {
            text = "CONFIGURACIÓN"
            typeface = bitcount
            setTextColor(Color.WHITE)
            textSize = 28f
            setPadding(0, 0, 0, 100)
        }
        controlsLayout.addView(titleMenu)

        val vhsSwitch = Switch(this).apply {
            text = "MODO VHS    "
            typeface = bitcount
            setTextColor(Color.WHITE)
            textSize = 20f
            isChecked = isVHSEnabledNative()
            setPadding(0, 0, 0, 80)
            setOnCheckedChangeListener { _, checked -> setVHSEffectNative(checked) }
        }
        controlsLayout.addView(vhsSwitch)

        val btnPickColor = Button(this).apply {
            text = "ELEGIR COLOR"
            typeface = bitcount
            setTextColor(Color.WHITE)
            setBackgroundColor("#1F1F1F".toColorInt())
            setPadding(40, 40, 40, 40)
            setOnClickListener { showProColorPicker(bitcount) }
        }
        controlsLayout.addView(btnPickColor)

        val btnClose = Button(this).apply {
            text = "VOLVER"
            typeface = bitcount
            setTextColor(Color.GRAY)
            setBackgroundColor(Color.TRANSPARENT)
            setOnClickListener { configDialog.dismiss() }
        }
        val closeParams = FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT,
            Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
        ).apply { setMargins(0, 0, 0, 100) }

        rootLayout.addView(controlsLayout)
        rootLayout.addView(btnClose, closeParams)

        configDialog.setContentView(rootLayout)
        configDialog.show()
    }

    private fun showProColorPicker(bitcountFont: Typeface?) {
        val builder = ColorPickerDialog.Builder(this)
            .setTitle("COLOR DE LA MASCOTA")
            .setPreferenceName("MyColorPicker")
            .setPositiveButton("Confirmar", ColorEnvelopeListener { envelope, _ ->
                currentColorInt = envelope.color

                val r = Color.red(currentColorInt) / 255f
                val g = Color.green(currentColorInt) / 255f
                val b = Color.blue(currentColorInt) / 255f

                setPetColorNative(r, g, b)
            })
            .setNegativeButton("Cancelar") { d, _ -> d.dismiss() }
            .attachAlphaSlideBar(false)
            .attachBrightnessSlideBar(true)
            .apply {
                val colorPickerView = colorPickerView
                colorPickerView.setInitialColor(currentColorInt)
                colorPickerView.setBackgroundColor("#121212".toColorInt())
            }

        val dialog = builder.create()
        dialog.window?.setBackgroundDrawable("#121212".toColorInt().toDrawable())
        dialog.show()

        val titleId = resources.getIdentifier("alertTitle", "id", "android")
        if (titleId > 0) {
            dialog.findViewById<TextView>(titleId)?.apply {
                typeface = bitcountFont
                setTextColor(Color.WHITE)
            }
        }

        dialog.getButton(AlertDialog.BUTTON_POSITIVE).apply {
            setTextColor(Color.CYAN)
            typeface = bitcountFont
        }
        dialog.getButton(AlertDialog.BUTTON_NEGATIVE).apply {
            setTextColor(Color.GRAY)
            typeface = bitcountFont
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideSystemUi()
    }

    private fun hideSystemUi() {
        window.decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN)
    }
}