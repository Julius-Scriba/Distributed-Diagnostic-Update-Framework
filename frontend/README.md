# ULTSPY Web UI

This web interface allows operators to interact with the ULTSPY C2 backend.
It is built with Vite, React, TypeScript and TailwindCSS.
The UI components follow the shadcn/ui style guidelines.

## Setup

```bash
npm install
npm run dev
```

The development server proxies API requests to `http://localhost:5000`.
Store your API key after logging in; requests include it via the `X-API-KEY` header.

## Project Structure

- `src/layout` contains the shared `Header`, `Sidebar` and `Layout` components.
- `src/pages` defines the initial routes:
  - **Dashboard** – landing page
  - **Agents** – list of connected agents
  - **Commands** – placeholder for command management
  - **Logs** – placeholder for log viewing
  - **Settings** – placeholder for future settings

Navigation is handled via React Router DOM and styled with TailwindCSS.
