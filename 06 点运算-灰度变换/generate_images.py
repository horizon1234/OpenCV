#!/usr/bin/env python3
"""Generate educational images for Gamma correction README."""

import os
import numpy as np
import matplotlib
matplotlib.use('Agg')  # Non-interactive backend
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap

# --- Global style ---
plt.rcParams.update({
    'figure.facecolor': 'white',
    'axes.facecolor': '#FAFAFA',
    'axes.grid': True,
    'grid.alpha': 0.3,
    'font.size': 12,
})

OUT_DIR = os.path.dirname(os.path.abspath(__file__))
IMG_DIR = os.path.join(OUT_DIR, 'images')
os.makedirs(IMG_DIR, exist_ok=True)


# ============================================================
# 1. Weber-Fechner perceptual curve
# ============================================================
def gen_weber_fechner():
    fig, ax = plt.subplots(figsize=(8, 5))
    I = np.linspace(0.01, 1.0, 500)
    S = np.log(I)
    S = (S - S.min()) / (S.max() - S.min())  # normalize to 0~1

    ax.plot(I * 100, S * 100, linewidth=2.5, color='#E74C3C',
            label=r'$S = k \cdot \ln(I)$  (perceptual)')
    ax.plot([0, 100], [0, 100], '--', linewidth=1.5, color='#999',
            label='Linear (ideal)')

    # Annotations
    ax.annotate('Dark region:\nSmall ΔI → Large ΔS\n(very sensitive)',
                xy=(10, 50), fontsize=10, color='#2C3E50',
                bbox=dict(boxstyle='round,pad=0.4', fc='#FFF9C4', ec='#F9A825'))
    ax.annotate('Bright region:\nLarge ΔI → Small ΔS\n(insensitive)',
                xy=(60, 82), fontsize=10, color='#2C3E50',
                bbox=dict(boxstyle='round,pad=0.4', fc='#E3F2FD', ec='#42A5F5'))

    ax.set_xlabel('Physical Brightness  I  (%)', fontsize=13)
    ax.set_ylabel('Perceived Brightness  S  (%)', fontsize=13)
    ax.set_title('Weber-Fechner Law: Human Brightness Perception', fontsize=14, fontweight='bold')
    ax.legend(loc='lower right', fontsize=11)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 105)

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '01_weber_fechner.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 01_weber_fechner.png')


# ============================================================
# 2. Linear gradient vs perceptual gradient
# ============================================================
def gen_gradient_bars():
    fig, axes = plt.subplots(3, 1, figsize=(10, 4))

    width = 512
    linear = np.linspace(0, 1, width).reshape(1, -1)

    # (a) Linear encoding
    axes[0].imshow(np.repeat(linear, 60, axis=0), cmap='gray',
                   aspect='auto', vmin=0, vmax=1)
    axes[0].set_title('(a) Linear Encoding  —  Physically uniform gradient',
                      fontsize=11, fontweight='bold', loc='left')
    axes[0].set_yticks([])
    axes[0].set_xticks([0, width//4, width//2, 3*width//4, width-1],
                       ['0%', '25%', '50%', '75%', '100%'])

    # (b) Gamma 2.2 applied (simulates CRT output of linear data)
    gamma_out = linear ** 2.2
    axes[1].imshow(np.repeat(gamma_out, 60, axis=0), cmap='gray',
                   aspect='auto', vmin=0, vmax=1)
    axes[1].set_title('(b) After CRT γ=2.2  —  Dark region crushed, overall too dark',
                      fontsize=11, fontweight='bold', loc='left')
    axes[1].set_yticks([])
    axes[1].set_xticks([0, width//4, width//2, 3*width//4, width-1],
                       ['0%', '25%', '50%', '75%', '100%'])

    # (c) Gamma encoded (1/2.2) then displayed on CRT (2.2) → linear again
    gamma_enc = linear ** (1/2.2)
    axes[2].imshow(np.repeat(gamma_enc, 60, axis=0), cmap='gray',
                   aspect='auto', vmin=0, vmax=1)
    axes[2].set_title('(c) Gamma-encoded γ=1/2.2  —  Brighter file, CRT restores linear',
                      fontsize=11, fontweight='bold', loc='left')
    axes[2].set_yticks([])
    axes[2].set_xticks([0, width//4, width//2, 3*width//4, width-1],
                       ['0%', '25%', '50%', '75%', '100%'])

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '02_gradient_bars.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 02_gradient_bars.png')


# ============================================================
# 3. CRT response curve (V^2.2)
# ============================================================
def gen_crt_response():
    fig, ax = plt.subplots(figsize=(8, 5))
    V = np.linspace(0, 1, 500)

    ax.plot(V * 100, (V ** 2.2) * 100, linewidth=2.5, color='#E74C3C',
            label=r'CRT:  $L = V^{2.2}$')
    ax.plot(V * 100, V * 100, '--', linewidth=1.5, color='#999', label='Linear (ideal)')
    ax.plot(V * 100, (V ** (1/2.2)) * 100, linewidth=2.5, color='#2196F3',
            label=r'Gamma encode:  $I\prime = V^{1/2.2}$')

    # Fill the "dark crush" region
    ax.fill_between(V * 100, (V ** 2.2) * 100, V * 100,
                    where=(V <= 0.5), alpha=0.12, color='red')
    ax.annotate('Dark region\ncrushed by CRT',
                xy=(20, 5), fontsize=10, color='#C62828',
                bbox=dict(boxstyle='round,pad=0.3', fc='#FFEBEE', ec='#E57373'))

    ax.set_xlabel('Input Voltage / Signal  V  (%)', fontsize=13)
    ax.set_ylabel('Output Brightness  L  (%)', fontsize=13)
    ax.set_title('Display Gamma Response & Correction', fontsize=14, fontweight='bold')
    ax.legend(loc='upper left', fontsize=11)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 105)

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '03_crt_response.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 03_crt_response.png')


# ============================================================
# 4. Multiple gamma curves
# ============================================================
def gen_gamma_curves():
    fig, ax = plt.subplots(figsize=(8, 6))
    x = np.linspace(0, 255, 500)
    xn = x / 255.0

    gammas = [0.2, 0.4, 0.6, 1.0, 1.8, 2.5, 4.0]
    colors = ['#F44336', '#FF9800', '#FFC107', '#4CAF50', '#2196F3', '#3F51B5', '#9C27B0']

    for g, c in zip(gammas, colors):
        y = 255 * (xn ** g)
        lw = 3.0 if g == 1.0 else 2.0
        ls = '--' if g == 1.0 else '-'
        ax.plot(x, y, color=c, linewidth=lw, linestyle=ls, label=f'γ = {g}')

    ax.set_xlabel('Input Pixel Value  I', fontsize=13)
    ax.set_ylabel('Output Pixel Value  I\'', fontsize=13)
    ax.set_title('Gamma Transform Curves', fontsize=14, fontweight='bold')
    ax.legend(fontsize=11, loc='lower right')
    ax.set_xlim(0, 255)
    ax.set_ylim(0, 260)

    # Region labels
    ax.annotate('γ < 1\nBrighten\n(enhance dark)',
                xy=(60, 200), fontsize=10, color='#BF360C',
                bbox=dict(boxstyle='round,pad=0.3', fc='#FFF3E0', ec='#FF8A65'))
    ax.annotate('γ > 1\nDarken\n(enhance bright)',
                xy=(160, 50), fontsize=10, color='#1A237E',
                bbox=dict(boxstyle='round,pad=0.3', fc='#E8EAF6', ec='#7986CB'))

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '04_gamma_curves.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 04_gamma_curves.png')


# ============================================================
# 5. JND (Just Noticeable Difference) visualization
# ============================================================
def gen_jnd_demo():
    fig, axes = plt.subplots(2, 5, figsize=(12, 5))

    # Dark pairs: base=30, differences of 1,3,5,8,12
    dark_base = 30
    dark_diffs = [1, 3, 5, 8, 15]
    for i, d in enumerate(dark_diffs):
        patch = np.zeros((100, 200), dtype=np.uint8)
        patch[:, :100] = dark_base
        patch[:, 100:] = dark_base + d
        axes[0, i].imshow(patch, cmap='gray', vmin=0, vmax=255)
        axes[0, i].set_title(f'{dark_base} vs {dark_base+d}\nΔ={d}', fontsize=9)
        axes[0, i].set_xticks([])
        axes[0, i].set_yticks([])

    axes[0, 0].set_ylabel('Dark Region\n(base=30)', fontsize=10, fontweight='bold')

    # Bright pairs: base=220, same differences
    bright_base = 220
    bright_diffs = [1, 3, 5, 8, 15]
    for i, d in enumerate(bright_diffs):
        patch = np.zeros((100, 200), dtype=np.uint8)
        patch[:, :100] = bright_base
        patch[:, 100:] = min(bright_base + d, 255)
        axes[1, i].imshow(patch, cmap='gray', vmin=0, vmax=255)
        axes[1, i].set_title(f'{bright_base} vs {min(bright_base+d,255)}\nΔ={d}', fontsize=9)
        axes[1, i].set_xticks([])
        axes[1, i].set_yticks([])

    axes[1, 0].set_ylabel('Bright Region\n(base=220)', fontsize=10, fontweight='bold')

    fig.suptitle('JND Demo: Same Δ is easier to see in dark regions',
                 fontsize=13, fontweight='bold', y=1.02)
    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '05_jnd_demo.png'), dpi=150, bbox_inches='tight')
    plt.close(fig)
    print('  ✓ 05_jnd_demo.png')


# ============================================================
# 6. Linear vs Gamma encoding bit allocation
# ============================================================
def gen_encoding_comparison():
    fig, axes = plt.subplots(2, 1, figsize=(10, 4.5))

    n = 256
    x_linear = np.arange(n)

    # (a) Linear encoding: uniform physical brightness
    linear_brightness = x_linear / 255.0
    bar_linear = linear_brightness.reshape(1, -1)
    axes[0].imshow(np.repeat(bar_linear, 50, axis=0), cmap='gray',
                   aspect='auto', vmin=0, vmax=1)
    axes[0].set_title('(a) Linear Encoding  —  256 levels spread evenly across physical brightness',
                      fontsize=11, fontweight='bold', loc='left')
    axes[0].set_yticks([])
    axes[0].set_xticks([0, 64, 128, 192, 255],
                       ['0', '64', '128', '192', '255'])
    # Draw bracket for dark region
    axes[0].axvline(x=50, color='red', linewidth=1.5, linestyle='--')
    axes[0].text(25, -12, '← Only 50 levels\nfor dark region',
                 fontsize=8, color='red', ha='center', va='top')

    # (b) Gamma encoded: more levels in dark region
    gamma_brightness = (x_linear / 255.0) ** (1/2.2)
    bar_gamma = gamma_brightness.reshape(1, -1)
    axes[1].imshow(np.repeat(bar_gamma, 50, axis=0), cmap='gray',
                   aspect='auto', vmin=0, vmax=1)
    axes[1].set_title('(b) Gamma Encoding (γ=1/2.2)  —  More levels allocated to dark region',
                      fontsize=11, fontweight='bold', loc='left')
    axes[1].set_yticks([])
    axes[1].set_xticks([0, 64, 128, 192, 255],
                       ['0', '64', '128', '192', '255'])
    axes[1].axvline(x=128, color='#2196F3', linewidth=1.5, linestyle='--')
    axes[1].text(64, -12, '← ~128 levels for\nsame dark range',
                 fontsize=8, color='#2196F3', ha='center', va='top')

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '06_encoding_comparison.png'), dpi=150,
                bbox_inches='tight')
    plt.close(fig)
    print('  ✓ 06_encoding_comparison.png')


# ============================================================
# 7. Gamma correction pipeline diagram
# ============================================================
def gen_pipeline():
    fig, ax = plt.subplots(figsize=(12, 4))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 4)
    ax.axis('off')

    boxes = [
        (0.5, 1.5, 2.2, 1.5, 'Real Scene\n(Linear Light)', '#E8F5E9', '#4CAF50'),
        (3.2, 1.5, 2.2, 1.5, 'Camera\n(Linear Capture)', '#FFF3E0', '#FF9800'),
        (5.9, 1.5, 2.2, 1.5, 'Gamma Encode\nγ = 1/2.2', '#E3F2FD', '#2196F3'),
        (8.6, 1.5, 2.2, 1.5, 'Display\nγ = 2.2', '#FCE4EC', '#E91E63'),
    ]

    for x, y, w, h, text, fc, ec in boxes:
        rect = plt.Rectangle((x, y), w, h, facecolor=fc, edgecolor=ec,
                              linewidth=2, zorder=2, clip_on=False)
        ax.add_patch(rect)
        ax.text(x + w/2, y + h/2, text, ha='center', va='center',
                fontsize=11, fontweight='bold', zorder=3)

    # Arrows
    arrow_kw = dict(arrowstyle='->', lw=2, color='#333')
    for x1, x2 in [(2.7, 3.2), (5.4, 5.9), (8.1, 8.6)]:
        ax.annotate('', xy=(x2, 2.25), xytext=(x1, 2.25),
                    arrowprops=arrow_kw)

    # Formulas below
    ax.text(6.0, 0.8, r"$I' = I^{1/2.2}$", fontsize=13, ha='center',
            color='#1565C0', style='italic')
    ax.text(9.7, 0.8, r"$L = I'^{2.2} = I^{1.0}$", fontsize=13, ha='center',
            color='#AD1457', style='italic')
    ax.text(9.7, 0.2, '→ Linear restored ✓', fontsize=11, ha='center',
            color='#2E7D32', fontweight='bold')

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '07_gamma_pipeline.png'), dpi=150,
                bbox_inches='tight')
    plt.close(fig)
    print('  ✓ 07_gamma_pipeline.png')


# ============================================================
# 8. Linear vs Gamma comparison (side by side transform demo)
# ============================================================
def gen_linear_vs_gamma():
    fig, axes = plt.subplots(1, 2, figsize=(10, 5))
    x = np.linspace(0, 255, 500)

    # (a) Linear transform
    ax = axes[0]
    y_linear = np.clip(1.5 * x + 20, 0, 255)
    ax.plot(x, x, '--', color='#999', linewidth=1.5, label='Identity')
    ax.plot(x, y_linear, linewidth=2.5, color='#F44336', label=r'$I\prime = 1.5I + 20$')
    ax.fill_between(x, 255, y_linear, where=(y_linear >= 255),
                    alpha=0.2, color='red')
    ax.annotate('Clipping!\nDetail lost', xy=(180, 258), fontsize=10,
                color='#C62828', ha='center',
                bbox=dict(boxstyle='round', fc='#FFEBEE', ec='#E57373'))
    ax.set_title('Linear Transform', fontsize=13, fontweight='bold')
    ax.set_xlabel('Input')
    ax.set_ylabel('Output')
    ax.legend(fontsize=10)
    ax.set_xlim(0, 255)
    ax.set_ylim(0, 280)

    # (b) Gamma transform
    ax = axes[1]
    xn = x / 255.0
    y_gamma = 255 * (xn ** 0.6)
    ax.plot(x, x, '--', color='#999', linewidth=1.5, label='Identity')
    ax.plot(x, y_gamma, linewidth=2.5, color='#2196F3', label=r'$I\prime = 255(I/255)^{0.6}$')
    ax.annotate('No clipping\nSmooth 0→255', xy=(100, 220), fontsize=10,
                color='#0D47A1', ha='center',
                bbox=dict(boxstyle='round', fc='#E3F2FD', ec='#64B5F6'))
    ax.set_title('Gamma Transform (γ=0.6)', fontsize=13, fontweight='bold')
    ax.set_xlabel('Input')
    ax.set_ylabel('Output')
    ax.legend(fontsize=10)
    ax.set_xlim(0, 255)
    ax.set_ylim(0, 280)

    fig.suptitle('Linear vs Gamma: Overflow Comparison', fontsize=14, fontweight='bold')
    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '08_linear_vs_gamma.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 08_linear_vs_gamma.png')


# ============================================================
# 9. LUT concept diagram
# ============================================================
def gen_lut_diagram():
    fig, ax = plt.subplots(figsize=(10, 5))

    x = np.arange(256)
    gamma = 0.6
    lut_vals = np.clip(255 * (x / 255.0) ** gamma, 0, 255).astype(int)

    ax.bar(x, lut_vals, width=1.0, color='#42A5F5', alpha=0.7, label='LUT output')
    ax.plot(x, x, '--', color='#999', linewidth=1.5, label='Identity (γ=1)')

    # Highlight a few lookups
    highlights = [(30, '#F44336'), (128, '#4CAF50'), (200, '#FF9800')]
    for val, col in highlights:
        out = lut_vals[val]
        ax.plot([val, val], [0, out], ':', color=col, linewidth=2)
        ax.plot([0, val], [out, out], ':', color=col, linewidth=2)
        ax.plot(val, out, 'o', color=col, markersize=8, zorder=5)
        ax.text(val + 3, out + 8, f'LUT[{val}] = {out}',
                fontsize=9, fontweight='bold', color=col)

    ax.set_xlabel('Input Pixel Value', fontsize=13)
    ax.set_ylabel('Output Pixel Value (from LUT)', fontsize=13)
    ax.set_title('LUT (Look-Up Table) for γ=0.6', fontsize=14, fontweight='bold')
    ax.legend(fontsize=11, loc='lower right')
    ax.set_xlim(0, 260)
    ax.set_ylim(0, 270)

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '09_lut_diagram.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 09_lut_diagram.png')


# ============================================================
# 10. Display technology comparison — native gamma curves
# ============================================================
def gen_display_tech_comparison():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5))

    V = np.linspace(0, 1, 500)

    # Left: native response curves
    crt = V ** 2.2
    lcd_native = V ** 1.1   # LCD native ≈ linear
    oled_native = V ** 1.8  # OLED native ≈ 1.8
    lcd_corrected = V ** 2.2  # After Gamma IC
    ideal = V

    ax1.plot(V, ideal, '--', color='#999', linewidth=1.5, label='Linear (γ=1)')
    ax1.plot(V, crt, linewidth=2.5, color='#E74C3C', label='CRT native (γ≈2.2)')
    ax1.plot(V, oled_native, linewidth=2.5, color='#9C27B0', label='OLED native (γ≈1.8)')
    ax1.plot(V, lcd_native, linewidth=2.5, color='#2196F3', label='LCD native (γ≈1.1)')
    ax1.plot(V, lcd_corrected, linewidth=2, color='#2196F3', linestyle=':',
             label='LCD + Gamma IC (γ→2.2)')

    ax1.set_xlabel('Input Signal (normalized)', fontsize=12)
    ax1.set_ylabel('Output Luminance (normalized)', fontsize=12)
    ax1.set_title('Native Gamma Response by Display Type', fontsize=13, fontweight='bold')
    ax1.legend(fontsize=9, loc='upper left')
    ax1.set_xlim(0, 1)
    ax1.set_ylim(0, 1)

    # Right: bar chart comparing properties
    technologies = ['CRT', 'LCD\n(TN/IPS/VA)', 'OLED', 'Mini-LED', 'Micro-LED']
    native_gamma = [2.2, 1.1, 1.8, 1.1, 1.3]
    colors = ['#E74C3C', '#2196F3', '#9C27B0', '#4CAF50', '#FF9800']

    bars = ax2.barh(technologies, native_gamma, color=colors, edgecolor='white', height=0.6)
    ax2.axvline(x=2.2, color='#E74C3C', linestyle='--', linewidth=1.5, alpha=0.7,
                label='Target γ=2.2')
    ax2.axvline(x=1.0, color='#999', linestyle=':', linewidth=1, alpha=0.5)

    for bar, val in zip(bars, native_gamma):
        ax2.text(bar.get_width() + 0.05, bar.get_y() + bar.get_height()/2,
                 f'γ ≈ {val}', va='center', fontsize=11, fontweight='bold')

    ax2.set_xlabel('Native Gamma Value', fontsize=12)
    ax2.set_title('Native Gamma by Technology\n(all corrected to 2.2 for display)', fontsize=13, fontweight='bold')
    ax2.set_xlim(0, 3.0)
    ax2.legend(fontsize=10)

    fig.tight_layout(pad=2.0)
    fig.savefig(os.path.join(IMG_DIR, '10_display_tech_comparison.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 10_display_tech_comparison.png')


# ============================================================
# 11. Gamma standards comparison chart
# ============================================================
def gen_gamma_standards():
    fig, ax = plt.subplots(figsize=(10, 5.5))

    V = np.linspace(0, 1, 500)

    # sRGB transfer function (piecewise)
    def srgb_encode(v):
        return np.where(v <= 0.0031308,
                        12.92 * v,
                        1.055 * v ** (1/2.4) - 0.055)

    srgb_enc = srgb_encode(V)
    gamma_22 = V ** (1/2.2)
    gamma_26_inv = V ** (1/2.6)  # DCI-P3 encoding
    gamma_18 = V ** (1/1.8)      # old Mac gamma

    ax.plot(V, V, '--', color='#999', linewidth=1.5, label='Linear (γ=1)')
    ax.plot(V, gamma_22, linewidth=2.5, color='#E74C3C',
            label='Simple Power γ=1/2.2 (Rec.709)')
    ax.plot(V, srgb_enc, linewidth=2.5, color='#2196F3', linestyle='-.',
            label='sRGB (piecewise, ≈γ=1/2.2)')
    ax.plot(V, gamma_26_inv, linewidth=2.5, color='#FF9800',
            label='DCI-P3 encoding γ=1/2.6')
    ax.plot(V, gamma_18, linewidth=2, color='#4CAF50', linestyle=':',
            label='Old Mac γ=1/1.8')

    # Annotate the difference between sRGB and simple power
    idx = 50
    ax.annotate('sRGB linear segment\n(dark region)',
                xy=(V[idx], srgb_enc[idx]), xytext=(0.15, 0.55),
                fontsize=9, color='#1565C0',
                arrowprops=dict(arrowstyle='->', color='#1565C0'),
                bbox=dict(boxstyle='round,pad=0.3', fc='#E3F2FD', ec='#42A5F5'))

    ax.set_xlabel('Linear Light (scene luminance)', fontsize=12)
    ax.set_ylabel('Encoded Value (stored in file)', fontsize=12)
    ax.set_title('Gamma Encoding Curves: Different Standards Compared', fontsize=13, fontweight='bold')
    ax.legend(fontsize=9, loc='lower right')
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1.05)

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '11_gamma_standards.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 11_gamma_standards.png')


# ============================================================
# 12. Gamma 2.2 historical timeline
# ============================================================
def gen_gamma_timeline():
    fig, ax = plt.subplots(figsize=(12, 4))
    ax.set_xlim(1945, 2035)
    ax.set_ylim(-1, 3)
    ax.axis('off')
    ax.set_title('The Story of Gamma 2.2: A Timeline', fontsize=14, fontweight='bold', pad=15)

    # Timeline bar
    ax.plot([1950, 2025], [1, 1], '-', color='#333', linewidth=3, solid_capstyle='round')

    events = [
        (1953, 'CRT invented\nγ≈2.2 (physics)', '#E74C3C', 2.2),
        (1965, 'γ≈2.2 matches\nhuman perception', '#9C27B0', 1.8),
        (1990, 'ITU Rec.709\n(HDTV standard)', '#1565C0', 2.4),
        (1996, 'sRGB standard\n(HP+Microsoft)', '#2196F3', 2.2),
        (2005, 'DCI-P3\n(cinema, γ=2.6)', '#FF9800', 1.8),
        (2007, 'LCD era\n(Gamma IC)', '#4CAF50', 2.4),
        (2015, 'OLED + HDR\n(PQ curve)', '#795548', 2.0),
        (2020, 'HDR10 / SDR\ncoexistence', '#607D8B', 2.2),
    ]

    for year, label, color, y_offset in events:
        ax.plot(year, 1, 'o', color=color, markersize=10, zorder=5)
        ax.plot([year, year], [1, y_offset], '-', color=color, linewidth=1.5)
        ax.text(year, y_offset + 0.1, f'{year}\n{label}', ha='center', va='bottom',
                fontsize=8, color=color, fontweight='bold',
                bbox=dict(boxstyle='round,pad=0.2', fc='white', ec=color, alpha=0.9))

    fig.tight_layout()
    fig.savefig(os.path.join(IMG_DIR, '12_gamma_timeline.png'), dpi=150)
    plt.close(fig)
    print('  ✓ 12_gamma_timeline.png')


# ============================================================
if __name__ == '__main__':
    print('Generating images...')
    gen_weber_fechner()
    gen_gradient_bars()
    gen_crt_response()
    gen_gamma_curves()
    gen_jnd_demo()
    gen_encoding_comparison()
    gen_pipeline()
    gen_linear_vs_gamma()
    gen_lut_diagram()
    gen_display_tech_comparison()
    gen_gamma_standards()
    gen_gamma_timeline()
    print(f'\nAll images saved to: {IMG_DIR}/')
