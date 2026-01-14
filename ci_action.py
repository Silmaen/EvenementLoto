#!/usr/bin/env python3
"""
Entry point for all the CI actions.
"""


def main():
    from argparse import ArgumentParser
    from ci.actions import get_actions
    from ci import log

    action_list = get_actions()

    parser = ArgumentParser("CI Action Selector")
    parser.add_argument(
        "action", type=str, choices=action_list.keys(), help="The CI action to perform."
    )
    parser.add_argument("preset", type=str, help="The preset to check.")
    args = parser.parse_args()

    if args.action not in action_list:
        log.error(f"Unknown action: {args.action}")
        return 1
    action_func = action_list[args.action]
    result = action_func(args.preset)
    if result != 0:
        log.error(f"Action '{args.action}' failed with exit code: {result}")
    else:
        log.info(f"Action '{args.action}' completed with result: {result}")
    return result


if __name__ == "__main__":
    exit(main())
