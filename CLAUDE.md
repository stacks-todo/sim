# STACKS Simulator - CLAUDE.md

## プロジェクト概要

インテリア型タスク管理デバイス「STACKS」の **LVGL C シミュレータ**。
SDL2 バックエンドを使い PC 上で動作し、実機の円形ディスプレイ UI を再現する。

本体のファームウェア（Svelte 5 + SvelteKit）は別リポジトリ → https://github.com/stacks-todo/STACKS

## 作業ルール

> **IMPORTANT: 以下のルールは必ず守ること。**

- **常に main ブランチで直接作業する。ブランチを作成してはいけない。**
- **コミット・プッシュは基本的に行わない**
- **作業開始前に必ず `CLAUDE.md` を読むこと**
- **実装内容に応じて `CLAUDE.md` を自分で適宜更新すること**

## 技術スタック

| レイヤ | 採用技術 |
|---|---|
| UI フレームワーク | LVGL v9.6.0-dev |
| 描画バックエンド | SDL2（PC シミュレーション） |
| 言語 | C99 |
| ビルドシステム | CMake 3.10+ |
| 数値演算 | libm（`-lm`） |

## ビルド方法

```bash
# ワークツリー内で実行
mkdir build && cd build
cmake .. && make -j$(nproc)
./bin/main
```

または Makefile ショートカット:

```bash
make
```

> **注意（ワークツリー）**: `lvgl/` サブモジュールがワークツリーでは空になる場合がある。
> CMakeLists.txt が自動的に `sim/lvgl/` にフォールバックするため、通常はそのままビルド可能。
> フォールバックが失敗する場合は以下を実行:
>
> ```bash
> git submodule update --init --recursive
> ```

## 画面構成

キーボードの **← → 矢印キー** で画面を切り替える。

| スクリーン | 内容 | キー操作 |
|---|---|---|
| Pomodoro | タイマー設定（作業時間・ループ回数の切替） | `Enter/Space` で状態切替 |
| Clock | アナログ時計（システム時刻をリアルタイム表示） | — |
| Stack | タスクのバブル可視化（静的） | — |
| Table | タスク一覧（スクロール可能） | `↑↓` でスクロール |

## ファイル構成

```
src/
  main.c              エントリポイント（480×480 SDL ウィンドウ）
  mouse_cursor_icon.c マウスカーソルアイコン（自動生成）
  stacks_app.h        カラー定数・スクリーン enum・共有宣言
  stacks_app.c        アプリ初期化・背景ヘルパー・キー操作・画面遷移
  anim_utils.h/c      cubic-bezier イージング・tx/ty/opa アニメーションヘルパー
  screen_clock.c      アナログ時計画面
  screen_pomodoro.c   ポモドーロタイマー画面
  screen_stack.c      バブル可視化画面
  screen_table.c      タスクリスト画面
lv_conf.h             LVGL 設定（フォント Montserrat 12–48 有効）
CMakeLists.txt        ビルド定義
```

## アニメーション設計

### カスタムイージング（`anim_utils.c`）

GSAP CustomEase の cubic-bezier を C（Newton–Raphson 法）で再実装。
LVGL の `lv_anim_path_cb_t` として差し込む。

| 名前 | 曲線 | 用途 |
|---|---|---|
| `stacks_ease_out` | `cubic-bezier(0.086, 0.875, 0.304, 1.0)` | 画面 Enter（速く入り、ゆっくり落ち着く） |
| `stacks_ease_in`  | `cubic-bezier(0.742, 0, 0.875, 0.322)` | 将来の Exit アニメ用（現在未使用） |

### 画面遷移（`stacks_app.c` の `stacks_go_to`）

`lv_screen_load_anim` を直接呼ぶ代わりに `stacks_go_to(next, dir)` を経由。
遷移前に次画面の `screen_xxx_anim_in(dir)` を呼び、要素を初期オフセット位置に
セットしてから LVGL のスライドアニメ（350 ms）と同時に EASE_OUT で収束させる。

| `dir` | 意味 |
|---|---|
| `+1` | 右キー / 左スワイプ（次画面へ、右から入る） |
| `-1` | 左キー / 右スワイプ（前画面へ、左から入る） |

### 各画面の entrance アニメーション

| 画面 | アニメ内容 |
|---|---|
| Pomodoro | UI パネル群が ±220 px 水平スライド；タスクカウントが下から 130 px 上昇 |
| Clock | 12 数字が ±220 px 水平スライド（12 ms stagger）；時刻/日付も同様；タスクカウント 130 px 上昇 |
| Stack | 日時ラベルが水平スライド；バブルが opacity stagger フェードイン（18 ms stagger） |
| Table | タスクカウントが 200 px 下から上昇；カードが opacity + y+15 stagger フェードイン（12 ms stagger） |

## デザイン仕様（Figma 準拠）

| 項目 | 値 |
|---|---|
| 画面解像度（シム） | 480 × 480 px |
| 実機解像度 | 720 × 720 px（円形） |
| カラーモード | ダークテーマ固定 |
| 背景色 | `#0D0D0D` |
| アウターリング | `#2B2B2B` |
| ミドルリング | `#1E1E1E` |
| インナー円 | `#121212` |
| アクセント赤 | `#E53935` |
| アクセントオレンジ | `#FF8C00` |
| アクセントブルー | `#29B6F6` |

## 再現できること / できないこと

### ✅ 再現できる

- 同心円リングによる「時計型」背景デザイン
- Pomodoro タイマー UI（ピル型セレクター・再生ボタン）
- アナログ時計（リアルタイム・時針・分針）
- バブル可視化（静的配置）
- タスクリスト（スクロール可能、モックデータ）
- キーボードでの画面切替
- カラースキーム全体（赤・オレンジ・青のアクセント）

### ❌ 再現できない（理由付き）

| 機能 | 理由 |
|---|---|
| 日本語テキスト | CJK フォントが lv_conf.h で無効（`LV_FONT_SOURCE_HAN_SANS_SC_14_CJK = 0`） → 英語表記に代替 |
| バブル物理演算 | Matter.js 相当のリジッドボディ物理はゼロから実装が必要 → 静的配置で代替 |
| GSAP アニメーション | LVGL の `lv_anim` は基本的で、複雑なイージングや演出は非対応 |
| テーブルの 3D カルーセル | LVGL に 3D 変換 API なし |
| Google Tasks 連携 | ネットワーク・OAuth なし → モックデータで代替 |
| 物理ロータリーエンコーダ | PC シムなのでキーボードで代替 |
| バブルの影・ハーフトーン効果 | LVGL の flat fill のみ → 単色で代替 |
| WiFi セットアップ・QR フロー | C シムには不要 |
| 多言語対応 | 実機 Svelte 側の機能 |
