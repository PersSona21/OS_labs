import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# ──────────────────────────────────────────────
# ДАННЫЕ
# ──────────────────────────────────────────────
T_seq = sum([439.13, 469.97, 454.62, 456.92, 452.18, 465.33, 464.47, 487.03, 470.55, 463.76]) / 10

print(T_seq)
threads_list = [1, 2, 4, 6, 8, 16, 64, 128, 512]
times_par    = [461.65, 240.34, 122.63, 100.69, 95.08, 96.17, 84.51, 83.95, 87.51]

# ──────────────────────────────────────────────
# ТАБЛИЦА
# ──────────────────────────────────────────────
df = pd.DataFrame({
    'Потоки': threads_list,
    'Время (мс)': times_par
})

df['Ускорение S']     = T_seq / df['Время (мс)']
df['Эффективность X'] = df['Ускорение S'] / df['Потоки']

df = df.round(2)

print("=== ТАБЛИЦА ===\n")
print(df.to_string(index=False))
print("\n" + "─" * 70 + "\n")

# ──────────────────────────────────────────────
# ГРАФИКИ — увеличенный размер + фокус на 1–16
# ──────────────────────────────────────────────
plt.style.use('seaborn-v0_8-whitegrid')

fig = plt.figure(figsize=(16, 10))          # значительно крупнее
gs = fig.add_gridspec(2, 2, height_ratios=[3, 2], hspace=0.32, wspace=0.28)

# График 1: всё, лог. шкала по X
ax1 = fig.add_subplot(gs[0, 0])
ax1.plot(threads_list, times_par, 'o-', color='#1f77b4', linewidth=2.4, markersize=8, label='Параллельное')
ax1.axhline(T_seq, color='#d62728', linestyle='--', linewidth=2.5, label=f'Последовательное  ({T_seq:.1f} мс)')
ax1.set_xscale('log')
ax1.set_xticks(threads_list)
ax1.set_xticklabels(threads_list, rotation=45)
ax1.set_xlabel('Количество потоков (лог. шкала)')
ax1.set_ylabel('Время, мс')
ax1.set_title('Время выполнения (полный диапазон)')
ax1.legend(loc='upper right', fontsize=10)
ax1.grid(True, alpha=0.5)

# График 2: Ускорение
ax2 = fig.add_subplot(gs[0, 1])
ax2.plot(threads_list, df['Ускорение S'], 'o-', color='#2ca02c', linewidth=2.4, markersize=8, label='Ускорение S')
ax2.axhline(1, color='gray', linestyle=':', linewidth=1.6)
ax2.set_xscale('log')
ax2.set_xticks(threads_list)
ax2.set_xticklabels(threads_list, rotation=45)
ax2.set_xlabel('Количество потоков (лог. шкала)')
ax2.set_ylabel('Ускорение')
ax2.set_title('Ускорение S = Ts / Tp')
ax2.legend(loc='lower right', fontsize=10)
ax2.grid(True, alpha=0.5)

# График 3: Zoom 1–16, ЛИНЕЙНАЯ шкала
ax3 = fig.add_subplot(gs[1, :])   # занимает всю нижнюю строку
mask = np.array(threads_list) <= 16
ax3.plot(np.array(threads_list)[mask], np.array(times_par)[mask], 'o-', 
         color='#1f77b4', linewidth=3, markersize=10, label='Параллельное')
ax3.axhline(T_seq, color='#d62728', linestyle='--', linewidth=3, 
            label=f'Последовательное  ({T_seq:.1f} мс)')
ax3.set_xlabel('Количество потоков (1–16)')
ax3.set_ylabel('Время, мс')
ax3.set_title('Детализация: 1–16 потоков (линейная шкала)')
ax3.set_xticks([1,2,4,6,8,16])
ax3.legend(loc='upper right', fontsize=11)
ax3.grid(True, alpha=0.6)

# Текстовая сводка справа внизу
ax_text = fig.add_subplot(gs[1, 1])
ax_text.axis('off')

max_s = df['Ускорение S'].max()
best_s = df.loc[df['Ускорение S'].idxmax(), 'Потоки']
best_x = df['Эффективность X'].max()
best_x_threads = df.loc[df['Эффективность X'].idxmax(), 'Потоки']

text = f"""Ключевые результаты (N=512, K=64)

Макс. ускорение:      {max_s:.2f} ×
   при {best_s} потоках

Макс. эффективность:  {best_x:.2f}
   при {best_x_threads} потоках

На 8 ядрах лучшее время ≈ {df.loc[df['Потоки']==8, 'Время (мс)'].values[0]} мс
При 64–128 потоках — небольшое улучшение
(вероятно, за счёт кэша / уменьшения false sharing)"""

ax_text.text(0.05, 0.92, text, fontsize=11, va='top', ha='left',
             bbox=dict(boxstyle='round,pad=0.6', fc='#f8f9fa', ec='#adb5bd'))

plt.suptitle('Анализ параллельной реализации (N=512, K=64)', fontsize=18, y=0.98)
plt.tight_layout()

plt.savefig('lab1_results_focus_1-16.png', dpi=360, bbox_inches='tight')
plt.show()

print("Сохранено: lab1_results_focus_1-16.png")