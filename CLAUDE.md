# STACKS - CLAUDE.md

## プロジェクト概要

インテリア型タスク管理デバイス「STACKS」のソフトウェア。Svelte 5 + SvelteKit + MasterCSS で構築された PWA。720x720px の固定ディスプレイで動作し、Raspberry Pi の物理ロータリーノブ＋ボタンで操作する。

**主目的**: Google Tasks（Google Todo）との連携によるタスク表示・操作。

詳細は [requirements.md](./requirements.md) を参照。

## プロダクト性質（重要）

このソフトウェアは**エンドユーザーが購入して使う製品**のファームウェアである。

- エンドユーザーは `.env` ファイルを直接編集しない・できない
- デバイスはキーボードなし・タッチなし・物理ノブのみで操作
- 初回設定（WiFi 接続・Google ログイン）はすべて**スマホ QR フロー**で完結させる
- `.env` の設定はデバイス製造時・デプロイ時にのみ行う（ユーザー操作ではない）

## 作業ルール

> **IMPORTANT: 以下のルールは必ず守ること。**

- **常に main ブランチで直接作業する。ブランチを作成してはいけない。**
- **コミット・プッシュは基本的に行わない**
- **npm パッケージを追加するときは、追加前に必ず確認を取ること**
- **実装内容に応じて `CLAUDE.md` と `requirements.md` を自分で適宜更新すること**

## 技術スタック

- **Framework**: Svelte 5 + SvelteKit 2
- **Styling**: MasterCSS (`@master/css`)
- **Animation**: GSAP 3
- **Runtime**: Node.js（`@sveltejs/adapter-node`）
- **Deployment**: adapter-node → SvelteKit の API エンドポイントが使用可能

## アーキテクチャ

### 認証・外部連携
- **Firebase は使わない**
- Google OAuth 2.0 を SvelteKit サーバーサイドで直接実装
- Google Tasks API へのアクセスは `/api/tasks/*` エンドポイント経由

### データフロー
```
Google Tasks API
      ↕ (OAuth 2.0)
SvelteKit API Routes (/api/tasks/*)
      ↕
LocalTask ストア（Svelte Store + ローカルキャッシュ）
      ↕
各ページコンポーネント
```

### 環境変数（`.env`）

> 以下はすべて**製造・デプロイ時**に設定。エンドユーザーは触らない。

```
GOOGLE_CLIENT_ID=
GOOGLE_CLIENT_SECRET=
GOOGLE_REDIRECT_URI=        # ローカルログイン用（開発時のみ・通常不要）
STACKS_WORKER_URL=          # Cloudflare Worker の URL（例: https://stacks-auth.xxx.workers.dev）
STACKS_WORKER_SECRET=       # Worker との共有シークレット
VITE_IS_PHYSICS=true        # 物理ノブ入力の有効化
VITE_CURSOR_VISIBLE=false   # Kioskモードでカーソル非表示
WIFI_AP_SSID=STACKS-Setup   # デバイス自身のホットスポット SSID（デフォルト可）
WIFI_AP_PASSWORD=stacks1234 # デバイス自身のホットスポットパスワード（デフォルト可）
```

### Cloudflare Worker（`worker/`）

ローカル IP に依存しない Google OAuth リレーサーバー。無料枠内に収まるよう 300 セッション/日のレートリミットを実装済み。

```
worker/
  src/index.ts    # Worker 本体
  wrangler.toml   # デプロイ設定（KV ID を記入してから使う）
  tsconfig.json
```

npm スクリプト:
- `npm run worker:login` — Cloudflare にログイン
- `npm run worker:kv-create` — KV Namespace 作成（初回のみ）
- `npm run worker:deploy` — デプロイ
- `npm run worker:secret` — シークレット設定

## 主要ファイル

| ファイル | 役割 |
|---|---|
| `src/lib/localTasks.ts` | タスクのローカルストア（localStorage）。Google Tasks と1対1対応設計済み |
| `src/lib/physicsController.ts` | 物理ノブ・ボタンの入力管理 |
| `src/lib/pomodoroStore.ts` | ポモドーロタイマー |
| `src/lib/languageStore.ts` | 多言語対応（ja/en/zh-Hans/zh-Hant/de/es/fr） |
| `src/routes/api/rotation/` | Raspberry Pi からの SSE イベント受信 |
| `src/routes/+layout.svelte` | SSE 接続・ページ遷移アニメーション |
| `src/routes/setup/+page.svelte` | WiFi 初期設定ページ（スマホブラウザ向け・`position:fixed` で全画面） |
| `src/routes/settings/wifi/+page.svelte` | デバイス AP の QR コード表示（720×720 デバイス画面） |
| `src/routes/api/setup/wifi/+server.ts` | WiFi 接続 API（`nmcli` 呼び出し） |

## WiFi 初期設定フロー

エンドユーザーがデバイスを購入後、初めて WiFi に接続するまでの流れ：

1. デバイス起動 → OS 側で AP（ホットスポット）モードを起動（`hostapd` + `dnsmasq` は OS 設定）
2. Settings → WiFi → デバイス自身の AP 用 QR コードを表示
3. スマホで QR スキャン → `STACKS-Setup` ホットスポットに接続
4. スマホブラウザで `http://192.168.4.1/setup` を開く
5. 家の WiFi の SSID とパスワードを入力して送信
6. `/api/setup/wifi` が `nmcli dev wifi connect <SSID> password <PASSWORD> ifname wlan0` を実行
7. デバイスが家の WiFi に接続完了 → AP モード終了

**ポイント**: AP モード (`hostapd` / `dnsmasq`) の起動は OS レベルの設定で行う。SvelteKit はセットアップ UI と `nmcli` 呼び出しのみ担当。

## Google Tasks 連携（実装予定）

### 追加予定ファイル
- `src/lib/server/googleTasks.ts` - Google Tasks API クライアント
- `src/lib/googleTasksStore.ts` - 同期状態を管理する Svelte ストア
- `src/routes/api/auth/login/+server.ts` - OAuth 開始
- `src/routes/api/auth/callback/+server.ts` - トークン取得
- `src/routes/api/auth/logout/+server.ts` - セッション削除
- `src/routes/api/tasks/+server.ts` - タスク一覧取得・作成
- `src/routes/api/tasks/[id]/+server.ts` - タスク更新・削除

### LocalTask ↔ Google Tasks フィールドマッピング
| LocalTask | Google Tasks |
|---|---|
| `id` | `id` |
| `title` | `title` |
| `description` | `notes` |
| `dueDate` | `due` (RFC 3339) |
| `status: 'pending'` | `status: 'needsAction'` |
| `status: 'completed'` | `status: 'completed'` |
| `priority`, `category`, `subtasks` | ローカルのみ（API に対応フィールドなし） |

## UI・表示の制約

- **固定解像度**: 720x720px（Raspberry Pi の物理ディスプレイ）
- **タッチ操作なし**: `touch-action: none`（物理ノブで操作）
- **カーソル非表示**: Kiosk モード
- **ダークテーマ**: デフォルト
- **例外**: `/setup` はスマホブラウザ向け。`position: fixed` でルートレイアウトの円を上書きし全画面表示する

## ページ構成

| ルート | 内容 |
|---|---|
| `/` | `/pomodoro` へリダイレクト |
| `/clock` | アナログ時計 + タスク数表示 |
| `/stack` | タスクのバブル可視化（物理演算） |
| `/pomodoro` | ポモドーロタイマー（ノブで操作） |
| `/table` | タスク一覧（3D回転カルーセル） |
| `/settings` | 言語設定・Google 認証・WiFi QR・タスクJSON編集 |
| `/settings/wifi` | デバイス AP の QR コード表示（WiFi 設定入口） |
| `/settings/qr` | Google Login QR コード表示 |
| `/setup` | WiFi 初期設定フォーム（スマホ向け・全画面） |
